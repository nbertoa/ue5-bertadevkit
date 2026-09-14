#include "GameplayTags/BertaGameplayTagUsageFinder.h"

#include "GameplayTags/BertaGameplayTagUsageFinderInternal.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityLibrary.h"
#include "Engine/Blueprint.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Modules/ModuleManager.h"
#include "UObject/UnrealType.h"

namespace BertaGameplayTagUsagePrivate
{
	FString NormalizeRoot(FString Root)
	{
		Root.ReplaceInline(TEXT("\\"), TEXT("/"));
		while (Root.Len() > 1 && Root.EndsWith(TEXT("/")))
		{
			Root.LeftChopInline(1, EAllowShrinking::No);
		}
		return Root;
	}

	bool IsUnderRoot(const FString& PackagePath, const FString& Root)
	{
		const FString NormalizedPath = NormalizeRoot(PackagePath);
		const FString NormalizedRoot = NormalizeRoot(Root);
		return NormalizedPath == NormalizedRoot || NormalizedPath.StartsWith(NormalizedRoot + TEXT("/"));
	}

	bool Matches(const FGameplayTag StoredTag, const FGameplayTag SearchTag, const EBertaGameplayTagMatchMode MatchMode)
	{
		if (StoredTag == SearchTag)
		{
			return true;
		}
		return MatchMode == EBertaGameplayTagMatchMode::ParentOrChild
			&& (StoredTag.MatchesTag(SearchTag) || SearchTag.MatchesTag(StoredTag));
	}

	bool IsProjectContentRoot(
		const FString& MountedRoot,
		const FString& LocalContentPath,
		const FString& ProjectDirectory)
	{
		const FString NormalizedRoot = NormalizeRoot(MountedRoot);
		if (NormalizedRoot == TEXT("/Game"))
		{
			return true;
		}
		if (!NormalizedRoot.StartsWith(TEXT("/")) || LocalContentPath.IsEmpty() || ProjectDirectory.IsEmpty())
		{
			return false;
		}

		FString FullContentPath = FPaths::ConvertRelativePathToFull(LocalContentPath);
		FString FullProjectDirectory = FPaths::ConvertRelativePathToFull(ProjectDirectory);
		FPaths::NormalizeDirectoryName(FullContentPath);
		FPaths::NormalizeDirectoryName(FullProjectDirectory);
		return FPaths::IsUnderDirectory(FullContentPath, FullProjectDirectory);
	}

	FString FormatSummary(
		const FString& TagName,
		const EBertaGameplayTagMatchMode MatchMode,
		const FSearchCounts& Counts)
	{
		const TCHAR* Status = Counts.bCanceled
			? TEXT("PartialCanceled")
			: Counts.FailedAssets > 0 ? TEXT("PartialLoadFailures") : TEXT("Complete");
		return FString::Printf(
			TEXT("Gameplay Tag usage search: Status=%s Tag=%s MatchMode=%s Candidates=%d Processed=%d Loaded=%d LoadFailed=%d Skipped=%d Results=%d"),
			Status,
			*TagName,
			MatchMode == EBertaGameplayTagMatchMode::Exact ? TEXT("Exact") : TEXT("ParentOrChild"),
			Counts.CandidateAssets,
			Counts.ProcessedAssets,
			Counts.LoadedAssets,
			Counts.FailedAssets,
			Counts.SkippedAssets,
			Counts.Results);
	}

	const TCHAR* UsageTypeName(const EBertaGameplayTagUsageType Type)
	{
		switch (Type)
		{
		case EBertaGameplayTagUsageType::ExactTag: return TEXT("ExactTag");
		case EBertaGameplayTagUsageType::Container: return TEXT("Container");
		case EBertaGameplayTagUsageType::Query: return TEXT("Query");
		default: return TEXT("Unknown");
		}
	}

	struct FScanner
	{
		FGameplayTag SearchTag;
		EBertaGameplayTagMatchMode MatchMode = EBertaGameplayTagMatchMode::Exact;
		FString AssetPath;
		UPackage* AssetPackage = nullptr;
		TArray<FBertaGameplayTagUsageResult>* Results = nullptr;
		TSet<const UObject*> VisitedObjects;

		void AddResult(
			const UObject& Object,
			const FString& PropertyPath,
			const EBertaGameplayTagUsageType UsageType,
			const FGameplayTag StoredTag) const
		{
			FBertaGameplayTagUsageResult& Result = Results->AddDefaulted_GetRef();
			Result.AssetPath = AssetPath;
			Result.ObjectPath = Object.GetPathName();
			Result.ClassPath = Object.GetClass()->GetPathName();
			Result.PropertyPath = PropertyPath;
			Result.UsageType = UsageType;
			Result.StoredTag = StoredTag;
			Result.Description = FString::Printf(
				TEXT("%s stores %s at %s"), UsageTypeName(UsageType), *StoredTag.ToString(), *PropertyPath);
		}

		void ScanProperty(
			const FProperty& Property,
			const void* Value,
			const UObject& ContextObject,
			const FString& PropertyPath)
		{
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
			{
				if (StructProperty->Struct == FGameplayTag::StaticStruct())
				{
					const FGameplayTag& StoredTag = *static_cast<const FGameplayTag*>(Value);
					if (Matches(StoredTag, SearchTag, MatchMode))
					{
						AddResult(ContextObject, PropertyPath, EBertaGameplayTagUsageType::ExactTag, StoredTag);
					}
					return;
				}
				if (StructProperty->Struct == FGameplayTagContainer::StaticStruct())
				{
					const FGameplayTagContainer& Container = *static_cast<const FGameplayTagContainer*>(Value);
					for (const FGameplayTag& StoredTag : Container.GetGameplayTagArray())
					{
						if (Matches(StoredTag, SearchTag, MatchMode))
						{
							AddResult(ContextObject, PropertyPath, EBertaGameplayTagUsageType::Container, StoredTag);
						}
					}
					return;
				}
				if (StructProperty->Struct == FGameplayTagQuery::StaticStruct())
				{
					const FGameplayTagQuery& Query = *static_cast<const FGameplayTagQuery*>(Value);
					for (const FGameplayTag& StoredTag : Query.GetGameplayTagArray())
					{
						if (Matches(StoredTag, SearchTag, MatchMode))
						{
							AddResult(ContextObject, PropertyPath, EBertaGameplayTagUsageType::Query, StoredTag);
						}
					}
					return;
				}

				for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
				{
					const FProperty& ChildProperty = **It;
					if (!ChildProperty.HasAnyPropertyFlags(CPF_Transient))
					{
						ScanProperty(
							ChildProperty,
							ChildProperty.ContainerPtrToValuePtr<void>(Value),
							ContextObject,
							PropertyPath + TEXT(".") + ChildProperty.GetName());
					}
				}
				return;
			}

			if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(&Property))
			{
				FScriptArrayHelper Helper(ArrayProperty, Value);
				for (int32 Index = 0; Index < Helper.Num(); ++Index)
				{
					ScanProperty(*ArrayProperty->Inner, Helper.GetRawPtr(Index), ContextObject, FString::Printf(TEXT("%s[%d]"), *PropertyPath, Index));
				}
				return;
			}

			if (const FSetProperty* SetProperty = CastField<FSetProperty>(&Property))
			{
				FScriptSetHelper Helper(SetProperty, Value);
				for (int32 Index = 0; Index < Helper.GetMaxIndex(); ++Index)
				{
					if (Helper.IsValidIndex(Index))
					{
						ScanProperty(*SetProperty->ElementProp, Helper.GetElementPtr(Index), ContextObject, FString::Printf(TEXT("%s[%d]"), *PropertyPath, Index));
					}
				}
				return;
			}

			if (const FMapProperty* MapProperty = CastField<FMapProperty>(&Property))
			{
				FScriptMapHelper Helper(MapProperty, Value);
				for (int32 Index = 0; Index < Helper.GetMaxIndex(); ++Index)
				{
					if (Helper.IsValidIndex(Index))
					{
						ScanProperty(*MapProperty->KeyProp, Helper.GetKeyPtr(Index), ContextObject, FString::Printf(TEXT("%s[%d].Key"), *PropertyPath, Index));
						ScanProperty(*MapProperty->ValueProp, Helper.GetValuePtr(Index), ContextObject, FString::Printf(TEXT("%s[%d].Value"), *PropertyPath, Index));
					}
				}
				return;
			}

			if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(&Property))
			{
				UObject* NestedObject = ObjectProperty->GetObjectPropertyValue(Value);
				if (NestedObject && !NestedObject->IsA<UClass>() && NestedObject->GetOutermost() == AssetPackage)
				{
					ScanObject(*NestedObject, PropertyPath);
				}
			}
		}

		void ScanObject(const UObject& Object, const FString& Prefix = FString())
		{
			if (VisitedObjects.Contains(&Object))
			{
				return;
			}
			VisitedObjects.Add(&Object);

			for (TFieldIterator<FProperty> It(Object.GetClass()); It; ++It)
			{
				const FProperty& Property = **It;
				if (Property.HasAnyPropertyFlags(CPF_Transient))
				{
					continue;
				}
				const FString PropertyPath = Prefix.IsEmpty()
					? Property.GetName()
					: Prefix + TEXT(".") + Property.GetName();
				ScanProperty(Property, Property.ContainerPtrToValuePtr<void>(&Object), Object, PropertyPath);
			}
		}
	};

	void GatherProjectContentRoots(TArray<FString>& OutRoots)
	{
		OutRoots.Reset();
		OutRoots.Add(TEXT("/Game"));

		for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPluginsWithContent())
		{
			if (Plugin->GetLoadedFrom() == EPluginLoadedFrom::Project && Plugin->IsMounted())
			{
				OutRoots.AddUnique(NormalizeRoot(Plugin->GetMountedAssetPath()));
			}
		}

		TArray<FString> MountedRoots;
		FPackageName::QueryRootContentPaths(MountedRoots);
		for (const FString& MountedRoot : MountedRoots)
		{
			FString LocalContentPath;
			if (FPackageName::TryConvertLongPackageNameToFilename(MountedRoot, LocalContentPath)
				&& IsProjectContentRoot(MountedRoot, LocalContentPath, FPaths::ProjectDir()))
			{
				OutRoots.AddUnique(NormalizeRoot(MountedRoot));
			}
		}
		OutRoots.Sort();
	}

	bool IsWithinProjectContent(const FString& PackagePath, const TArray<FString>& ProjectRoots)
	{
		return ProjectRoots.ContainsByPredicate(
			[&PackagePath](const FString& Root)
			{
				return IsUnderRoot(PackagePath, Root);
			});
	}

	void GatherCandidates(
		const EBertaGameplayTagUsageScope Scope,
		const FString& ExplicitGameRoot,
		const TArray<FString>& ProjectRoots,
		TArray<FAssetData>& OutAssets)
	{
		if (Scope == EBertaGameplayTagUsageScope::SelectedContentBrowserAssets)
		{
			for (const FAssetData& Asset : UEditorUtilityLibrary::GetSelectedAssetData())
			{
				if (IsWithinProjectContent(Asset.PackagePath.ToString(), ProjectRoots))
				{
					OutAssets.Add(Asset);
				}
			}
			return;
		}

		FARFilter Filter;
		if (Scope == EBertaGameplayTagUsageScope::WholeGame)
		{
			for (const FString& Root : ProjectRoots)
			{
				Filter.PackagePaths.Add(*Root);
			}
		}
		else
		{
			Filter.PackagePaths.Add(*NormalizeRoot(ExplicitGameRoot));
		}
		Filter.bRecursivePaths = true;
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().GetAssets(Filter, OutAssets);
	}
}

bool UBertaGameplayTagUsageFinder::FindGameplayTagUsages(
	const FGameplayTag Tag,
	const EBertaGameplayTagUsageScope Scope,
	const EBertaGameplayTagMatchMode MatchMode,
	const FString& ExplicitGameRoot,
	TArray<FBertaGameplayTagUsageResult>& OutResults,
	FString& OutSummary)
{
	OutResults.Reset();
	OutSummary.Reset();
	if (!Tag.IsValid())
	{
		OutSummary = TEXT("Search failed: Gameplay Tag is invalid.");
		return false;
	}

	TArray<FString> ProjectRoots;
	BertaGameplayTagUsagePrivate::GatherProjectContentRoots(ProjectRoots);
	if (Scope == EBertaGameplayTagUsageScope::ExplicitGameRoot
		&& !BertaGameplayTagUsagePrivate::IsWithinProjectContent(ExplicitGameRoot, ProjectRoots))
	{
		OutSummary = TEXT("Search failed: ExplicitGameRoot must be under a mounted project or project-plugin content root.");
		return false;
	}

	TArray<FAssetData> Assets;
	BertaGameplayTagUsagePrivate::GatherCandidates(Scope, ExplicitGameRoot, ProjectRoots, Assets);
	Assets.Sort([](const FAssetData& Left, const FAssetData& Right)
	{
		return Left.GetObjectPathString() < Right.GetObjectPathString();
	});

	BertaGameplayTagUsagePrivate::FSearchCounts Counts;
	Counts.CandidateAssets = Assets.Num();
	TArray<FString> FailedAssetExamples;
	constexpr int32 MaximumFailureExamples = 10;
	FScopedSlowTask Progress(
		Assets.Num(),
		FText::Format(
			NSLOCTEXT("BertaGameplayTagUsageFinder", "Scanning", "Scanning project content for Gameplay Tag {0}..."),
			FText::FromName(Tag.GetTagName())));
	if (Scope == EBertaGameplayTagUsageScope::WholeGame)
	{
		Progress.MakeDialog(true);
	}
	else
	{
		Progress.MakeDialogDelayed(0.5f, true);
	}

	for (const FAssetData& AssetData : Assets)
	{
		if (Progress.ShouldCancel())
		{
			Counts.bCanceled = true;
			break;
		}
		Progress.EnterProgressFrame(
			1.0f,
			FText::Format(
				NSLOCTEXT("BertaGameplayTagUsageFinder", "LoadingAsset", "Loading {0}"),
				FText::FromString(AssetData.GetObjectPathString())));
		++Counts.ProcessedAssets;

		if (AssetData.IsRedirector())
		{
			++Counts.SkippedAssets;
			continue;
		}

		UObject* Asset = AssetData.GetAsset();
		if (!Asset)
		{
			++Counts.FailedAssets;
			if (FailedAssetExamples.Num() < MaximumFailureExamples)
			{
				FailedAssetExamples.Add(AssetData.GetObjectPathString());
			}
			continue;
		}
		++Counts.LoadedAssets;

		BertaGameplayTagUsagePrivate::FScanner Scanner;
		Scanner.SearchTag = Tag;
		Scanner.MatchMode = MatchMode;
		Scanner.AssetPath = AssetData.GetObjectPathString();
		Scanner.AssetPackage = Asset->GetOutermost();
		Scanner.Results = &OutResults;
		Scanner.ScanObject(*Asset);

		if (const UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
			Blueprint && Blueprint->GeneratedClass)
		{
			if (const UObject* ClassDefaultObject = Blueprint->GeneratedClass->GetDefaultObject(false))
			{
				Scanner.ScanObject(*ClassDefaultObject, TEXT("ClassDefaultObject"));
			}
		}
	}

	OutResults.Sort([](const FBertaGameplayTagUsageResult& Left, const FBertaGameplayTagUsageResult& Right)
	{
		if (Left.AssetPath != Right.AssetPath)
		{
			return Left.AssetPath < Right.AssetPath;
		}
		if (Left.ObjectPath != Right.ObjectPath)
		{
			return Left.ObjectPath < Right.ObjectPath;
		}
		if (Left.PropertyPath != Right.PropertyPath)
		{
			return Left.PropertyPath < Right.PropertyPath;
		}
		if (Left.UsageType != Right.UsageType)
		{
			return Left.UsageType < Right.UsageType;
		}
		return Left.StoredTag.ToString() < Right.StoredTag.ToString();
	});

	Counts.Results = OutResults.Num();
	OutSummary = BertaGameplayTagUsagePrivate::FormatSummary(Tag.ToString(), MatchMode, Counts);
	if (Counts.FailedAssets > 0)
	{
		UE_LOG(
			LogBertaDevKitEditor,
			Warning,
			TEXT("Gameplay Tag usage search could not load %d assets. First %d: %s"),
			Counts.FailedAssets,
			FailedAssetExamples.Num(),
			*FString::Join(FailedAssetExamples, TEXT(", ")));
	}
	if (Counts.bCanceled)
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("%s"), *OutSummary);
	}
	else
	{
		UE_LOG(LogBertaDevKitEditor, Log, TEXT("%s"), *OutSummary);
	}
	return !Counts.bCanceled && Counts.FailedAssets == 0;
}
