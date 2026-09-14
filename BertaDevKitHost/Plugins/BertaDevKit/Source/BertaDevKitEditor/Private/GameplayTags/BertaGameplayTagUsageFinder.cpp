#include "GameplayTags/BertaGameplayTagUsageFinder.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityLibrary.h"
#include "Engine/Blueprint.h"
#include "UObject/UnrealType.h"

namespace BertaGameplayTagUsagePrivate
{
	bool IsGamePath(const FString& Path)
	{
		return Path == TEXT("/Game") || Path.StartsWith(TEXT("/Game/"));
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

	void GatherCandidates(const EBertaGameplayTagUsageScope Scope, const FString& ExplicitGameRoot, TArray<FAssetData>& OutAssets)
	{
		if (Scope == EBertaGameplayTagUsageScope::SelectedContentBrowserAssets)
		{
			for (const FAssetData& Asset : UEditorUtilityLibrary::GetSelectedAssetData())
			{
				if (IsGamePath(Asset.PackagePath.ToString()))
				{
					OutAssets.Add(Asset);
				}
			}
			return;
		}

		const FString RootPath = Scope == EBertaGameplayTagUsageScope::WholeGame ? TEXT("/Game") : ExplicitGameRoot;
		if (!IsGamePath(RootPath))
		{
			return;
		}

		FARFilter Filter;
		Filter.PackagePaths.Add(*RootPath);
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
	if (Scope == EBertaGameplayTagUsageScope::ExplicitGameRoot && !BertaGameplayTagUsagePrivate::IsGamePath(ExplicitGameRoot))
	{
		OutSummary = TEXT("Search failed: ExplicitGameRoot must be /Game or a /Game/... path.");
		return false;
	}

	TArray<FAssetData> Assets;
	BertaGameplayTagUsagePrivate::GatherCandidates(Scope, ExplicitGameRoot, Assets);
	Assets.Sort([](const FAssetData& Left, const FAssetData& Right)
	{
		return Left.GetObjectPathString() < Right.GetObjectPathString();
	});

	int32 LoadedAssetCount = 0;
	for (const FAssetData& AssetData : Assets)
	{
		if (AssetData.IsRedirector())
		{
			continue;
		}

		UObject* Asset = AssetData.GetAsset();
		if (!Asset)
		{
			continue;
		}
		++LoadedAssetCount;

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
		return Left.PropertyPath < Right.PropertyPath;
	});

	OutSummary = FString::Printf(
		TEXT("Gameplay Tag usage search: Tag=%s MatchMode=%s AssetsLoaded=%d Results=%d"),
		*Tag.ToString(),
		MatchMode == EBertaGameplayTagMatchMode::Exact ? TEXT("Exact") : TEXT("ParentOrChild"),
		LoadedAssetCount,
		OutResults.Num());
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("%s"), *OutSummary);
	for (const FBertaGameplayTagUsageResult& Result : OutResults)
	{
		UE_LOG(LogBertaDevKitEditor, Log, TEXT("%s | %s | %s"), *Result.AssetPath, *Result.PropertyPath, *Result.Description);
	}
	return true;
}
