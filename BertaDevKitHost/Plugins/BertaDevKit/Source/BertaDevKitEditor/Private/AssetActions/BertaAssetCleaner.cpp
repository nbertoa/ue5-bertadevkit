#include "AssetActions/BertaAssetCleaner.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetIdentifier.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetViewUtils.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackagePath.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/ObjectRedirector.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "WorldPartition/DataLayer/ExternalDataLayerHelper.h"

#define LOCTEXT_NAMESPACE "BertaAssetCleaner"

namespace
{
	struct FInspectionResult
	{
		TArray<FBertaAssetCleanerPackageRecord> Packages;
		FBertaAssetCleanerGraphAnalysis Graph;
	};

	bool ContainsPackagePathSegment(const FString& PackageName, const TCHAR* Segment)
	{
		const FString SegmentWithSeparators = FString::Printf(TEXT("/%s/"), Segment);
		return PackageName.Contains(SegmentWithSeparators, ESearchCase::IgnoreCase)
			|| PackageName.EndsWith(FString::Printf(TEXT("/%s"), Segment), ESearchCase::IgnoreCase);
	}

	bool IsGeneratedWorldStoragePackage(const FAssetData& AssetData)
	{
		const FString PackageName = AssetData.PackageName.ToString();
		return ContainsPackagePathSegment(PackageName, FPackagePath::GetExternalActorsFolderName())
			|| ContainsPackagePathSegment(PackageName, FPackagePath::GetExternalObjectsFolderName())
			|| FExternalDataLayerHelper::IsExternalDataLayerPath(PackageName);
	}

	void ShowAssetCleanerNotification(const FText& Message, SNotificationItem::ECompletionState State)
	{
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 4.0f;
		if (const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Item->SetCompletionState(State);
		}
	}

	bool InspectAssets(const TArray<FAssetData>& Assets, const TCHAR* OperationName, FInspectionResult& OutResult)
	{
		OutResult = FInspectionResult();
		IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		if (Registry.IsGathering())
		{
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] %s aborted because the Asset Registry is gathering."), OperationName);
			ShowAssetCleanerNotification(LOCTEXT("RegistryGathering", "Asset Cleaner cannot run while the Asset Registry is gathering. Try again when scanning completes."), SNotificationItem::CS_None);
			return false;
		}

		TMap<FName, int32> PackageIndices;
		for (const FAssetData& Asset : Assets)
		{
			int32* ExistingIndex = PackageIndices.Find(Asset.PackageName);
			if (!ExistingIndex)
			{
				const int32 NewIndex = OutResult.Packages.AddDefaulted();
				OutResult.Packages[NewIndex].PackageName = Asset.PackageName;
				PackageIndices.Add(Asset.PackageName, NewIndex);
				ExistingIndex = PackageIndices.Find(Asset.PackageName);
			}
			OutResult.Packages[*ExistingIndex].Assets.Add(Asset);
		}

		UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
		for (FBertaAssetCleanerPackageRecord& Package : OutResult.Packages)
		{
			for (const FAssetData& Asset : Package.Assets)
			{
				FBertaAssetCleanerInspection Inspection;
				Inspection.bReferencerQuerySucceeded = true;
				Inspection.bPrimaryAssetQueryAvailable = AssetManager && AssetManager->HasInitialScanCompleted();
				Inspection.bIsRegisteredPrimaryAsset = Inspection.bPrimaryAssetQueryAvailable && AssetManager->GetPrimaryAssetIdForPath(Asset.GetSoftObjectPath()).IsValid();
				const FBertaAssetCleanerClassificationResult Classification = FBertaAssetCleaner::ClassifyAsset(Asset, Inspection);
				if (Classification.Classification == EBertaAssetCleanerClassification::Protected)
				{
					Package.bProtected = true;
					Package.Reason = Classification.Reason;
				}
				else if (Classification.Classification == EBertaAssetCleanerClassification::Skipped)
				{
					Package.bSkipped = true;
					Package.Reason = Classification.Reason;
				}
			}

			TArray<FAssetIdentifier> Referencers;
			if (!Registry.GetReferencers(FAssetIdentifier(Package.PackageName), Referencers, UE::AssetRegistry::EDependencyCategory::All, UE::AssetRegistry::FDependencyQuery()))
			{
				Package.bSkipped = true;
				Package.Reason = TEXT("Asset Registry referencer query failed");
			}
			else
			{
				for (const FAssetIdentifier& Referencer : Referencers)
				{
					if (Referencer.PackageName.IsNone())
					{
						Package.bHasExternalReferencer = true;
					}
					else if (Referencer.PackageName != Package.PackageName)
					{
						Package.ReferencerPackages.Add(Referencer.PackageName);
						if (!PackageIndices.Contains(Referencer.PackageName))
						{
							Package.bHasExternalReferencer = true;
						}
					}
				}
			}

			TArray<FAssetIdentifier> Dependencies;
			if (!Registry.GetDependencies(FAssetIdentifier(Package.PackageName), Dependencies, UE::AssetRegistry::EDependencyCategory::All, UE::AssetRegistry::FDependencyQuery()))
			{
				Package.bDependencyQuerySucceeded = false;
				OutResult.Graph.bComplete = false;
			}
			else
			{
				for (const FAssetIdentifier& Dependency : Dependencies)
				{
					if (Dependency.PackageName.IsNone())
					{
						OutResult.Graph.bComplete = false;
					}
					else if (Dependency.PackageName != Package.PackageName)
					{
						Package.DependencyPackages.Add(Dependency.PackageName);
					}
				}
			}
		}

		const FBertaAssetCleanerGraphAnalysis ComputedGraph = FBertaAssetCleaner::AnalyzePackageGraph(OutResult.Packages);
		OutResult.Graph.LivePackages = ComputedGraph.LivePackages;
		OutResult.Graph.OrphanPackages = ComputedGraph.OrphanPackages;
		OutResult.Graph.OrphanGroups = ComputedGraph.OrphanGroups;
		OutResult.Graph.bComplete = OutResult.Graph.bComplete && ComputedGraph.bComplete;
		return true;
	}

	void LogResults(const FInspectionResult& Result, const TCHAR* Operation)
	{
		int32 ProtectedCount = 0;
		int32 SkippedCount = 0;
		for (const FBertaAssetCleanerPackageRecord& Package : Result.Packages)
		{
			if (Package.bProtected)
			{
				++ProtectedCount;
				UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] PROTECTED: %s - %s"), *Package.PackageName.ToString(), *Package.Reason);
			}
			if (Package.bSkipped)
			{
				++SkippedCount;
				UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - %s"), *Package.PackageName.ToString(), *Package.Reason);
			}
		}
		if (!Result.Graph.bComplete)
		{
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] %s graph analysis is incomplete; graph-derived orphan candidates are unavailable."), Operation);
		}
		for (int32 GroupIndex = 0; GroupIndex < Result.Graph.OrphanGroups.Num(); ++GroupIndex)
		{
			const TArray<FName>& Group = Result.Graph.OrphanGroups[GroupIndex];
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] ORPHAN GROUP %d: %d package(s)"), GroupIndex + 1, Group.Num());
			for (const FName PackageName : Group)
			{
				UE_LOG(LogBertaDevKitEditor, Warning, TEXT("  %s"), *PackageName.ToString());
			}
		}
		UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] %s complete: %d orphan group(s), %d orphan candidate package(s), %d live package(s), %d protected, %d skipped."), Operation, Result.Graph.OrphanGroups.Num(), Result.Graph.OrphanPackages.Num(), Result.Graph.LivePackages.Num(), ProtectedCount, SkippedCount);
	}

	void CollectCandidateAssets(const FInspectionResult& Result, TArray<FAssetData>& OutCandidates)
	{
		OutCandidates.Reset();
		if (!Result.Graph.bComplete)
		{
			return;
		}
		for (const FBertaAssetCleanerPackageRecord& Package : Result.Packages)
		{
			if (Result.Graph.OrphanPackages.Contains(Package.PackageName))
			{
				OutCandidates.Append(Package.Assets);
			}
		}
	}

	FString NormalizeFolderPath(FString FolderPath)
	{
		FolderPath.ReplaceInline(TEXT("\\"), TEXT("/"));
		while (FolderPath.Len() > 1 && FolderPath.EndsWith(TEXT("/")))
		{
			FolderPath.LeftChopInline(1);
		}
		return FolderPath;
	}

	bool IsProjectFolderPath(const FString& FolderPath)
	{
		return FolderPath == TEXT("/Game") || FolderPath.StartsWith(TEXT("/Game/"));
	}

	bool TryGetProjectContentDirectory(const FString& FolderPath, FString& OutDirectory)
	{
		if (!IsProjectFolderPath(FolderPath) || !FPackageName::TryConvertLongPackageNameToFilename(FolderPath, OutDirectory))
		{
			return false;
		}

		FPaths::NormalizeDirectoryName(OutDirectory);
		FString ProjectContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
		FPaths::NormalizeDirectoryName(ProjectContentDirectory);
		return OutDirectory == ProjectContentDirectory || FPaths::IsUnderDirectory(OutDirectory, ProjectContentDirectory);
	}

	bool TryGetProjectFolderPath(const FString& Directory, FString& OutFolderPath)
	{
		FString NormalizedDirectory = Directory;
		FPaths::NormalizeDirectoryName(NormalizedDirectory);
		if (!FPackageName::TryConvertFilenameToLongPackageName(NormalizedDirectory, OutFolderPath))
		{
			return false;
		}

		OutFolderPath = NormalizeFolderPath(MoveTemp(OutFolderPath));
		return IsProjectFolderPath(OutFolderPath);
	}

	bool DoesDirectoryTreeContainFiles(const FString& Directory, bool& bOutEnumerated)
	{
		bOutEnumerated = false;
		if (!FPaths::DirectoryExists(Directory))
		{
			return true;
		}

		bool bContainsFiles = false;
		const bool bIterationCompleted = IFileManager::Get().IterateDirectoryRecursively(*Directory, [&bContainsFiles](const TCHAR*, const bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				bContainsFiles = true;
				return false;
			}
			return true;
		});
		bOutEnumerated = bIterationCompleted || bContainsFiles;
		return bContainsFiles;
	}

	void AddFolderAndDescendants(const FString& FolderPath, const FString& Directory, TSet<FString>& InOutFolderPaths, int32& InOutDirectoriesInspected, int32& InOutSkippedDirectories)
	{
		auto AddDirectory = [&InOutFolderPaths, &InOutDirectoriesInspected, &InOutSkippedDirectories](const FString& PhysicalDirectory, const FString* KnownFolderPath = nullptr)
		{
			FString CandidateFolderPath;
			if (KnownFolderPath)
			{
				CandidateFolderPath = *KnownFolderPath;
			}
			else if (!TryGetProjectFolderPath(PhysicalDirectory, CandidateFolderPath))
			{
				++InOutSkippedDirectories;
				UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - directory could not be mapped safely to /Game."), *PhysicalDirectory);
				return;
			}

			if (!InOutFolderPaths.Contains(CandidateFolderPath))
			{
				++InOutDirectoriesInspected;
				InOutFolderPaths.Add(CandidateFolderPath);
			}
		};

		AddDirectory(Directory, &FolderPath);
		IFileManager::Get().IterateDirectoryRecursively(*Directory, [&AddDirectory](const TCHAR* FilenameOrDirectory, const bool bIsDirectory)
		{
			if (bIsDirectory)
			{
				AddDirectory(FilenameOrDirectory);
			}
			return true;
		});
	}
}

FBertaAssetCleanerClassificationResult FBertaAssetCleaner::ClassifyAsset(const FAssetData& AssetData, const FBertaAssetCleanerInspection& Inspection)
{
	if (!Inspection.bReferencerQuerySucceeded) return { EBertaAssetCleanerClassification::Skipped, TEXT("Asset Registry referencer query failed") };
	if (Inspection.ReferencerCount > 0) return { EBertaAssetCleanerClassification::Referenced, FString() };
	if (AssetData.AssetClassPath == UWorld::StaticClass()->GetClassPathName()) return { EBertaAssetCleanerClassification::Protected, TEXT("World/map asset") };
	if (AssetData.AssetClassPath == UObjectRedirector::StaticClass()->GetClassPathName()) return { EBertaAssetCleanerClassification::Protected, TEXT("Object redirector") };
	if (IsGeneratedWorldStoragePackage(AssetData)) return { EBertaAssetCleanerClassification::Protected, TEXT("World Partition or external generated storage") };
	if (!Inspection.bPrimaryAssetQueryAvailable) return { EBertaAssetCleanerClassification::Skipped, TEXT("Asset Manager is unavailable; primary asset registration could not be checked") };
	if (Inspection.bIsRegisteredPrimaryAsset) return { EBertaAssetCleanerClassification::Protected, TEXT("Registered Primary Asset") };
	return { EBertaAssetCleanerClassification::UnusedCandidate, FString() };
}

FBertaAssetCleanerGraphAnalysis FBertaAssetCleaner::AnalyzePackageGraph(const TArray<FBertaAssetCleanerPackageRecord>& Records)
{
	FBertaAssetCleanerGraphAnalysis Result;
	TMap<FName, const FBertaAssetCleanerPackageRecord*> ByName;
	for (const FBertaAssetCleanerPackageRecord& Record : Records)
	{
		ByName.Add(Record.PackageName, &Record);
		Result.bComplete &= Record.bDependencyQuerySucceeded;
	}
	if (!Result.bComplete) return Result;

	TArray<FName> Queue;
	for (const FBertaAssetCleanerPackageRecord& Record : Records)
	{
		if (Record.bProtected || Record.bSkipped || Record.bHasExternalReferencer)
		{
			Result.LivePackages.Add(Record.PackageName);
			Queue.Add(Record.PackageName);
		}
	}
	for (int32 Index = 0; Index < Queue.Num(); ++Index)
	{
		const FBertaAssetCleanerPackageRecord* Record = ByName.FindChecked(Queue[Index]);
		for (const FName Dependency : Record->DependencyPackages)
		{
			if (ByName.Contains(Dependency) && !Result.LivePackages.Contains(Dependency))
			{
				Result.LivePackages.Add(Dependency);
				Queue.Add(Dependency);
			}
		}
	}
	for (const FBertaAssetCleanerPackageRecord& Record : Records)
	{
		if (!Record.bProtected && !Record.bSkipped && !Result.LivePackages.Contains(Record.PackageName)) Result.OrphanPackages.Add(Record.PackageName);
	}

	TSet<FName> Visited;
	TArray<FName> OrderedOrphans = Result.OrphanPackages.Array();
	OrderedOrphans.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });
	for (const FName Start : OrderedOrphans)
	{
		if (Visited.Contains(Start)) continue;
		Visited.Add(Start);
		TArray<FName> Group;
		TArray<FName> Work = { Start };
		for (int32 Index = 0; Index < Work.Num(); ++Index)
		{
			const FName Current = Work[Index];
			Group.Add(Current);
			for (const FBertaAssetCleanerPackageRecord& Other : Records)
			{
				const bool bConnected = Other.PackageName == Current || Other.DependencyPackages.Contains(Current) || ByName.FindChecked(Current)->DependencyPackages.Contains(Other.PackageName);
				if (bConnected && Result.OrphanPackages.Contains(Other.PackageName) && !Visited.Contains(Other.PackageName))
				{
					Visited.Add(Other.PackageName);
					Work.Add(Other.PackageName);
				}
			}
		}
		Group.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });
		Result.OrphanGroups.Add(MoveTemp(Group));
	}
	return Result;
}

void FBertaAssetCleaner::AuditUnusedAssets(const TArray<FAssetData>& Assets)
{
	FInspectionResult Result;
	if (!InspectAssets(Assets, TEXT("Audit"), Result)) return;
	LogResults(Result, TEXT("Audit"));
	ShowAssetCleanerNotification(Result.Graph.bComplete ? FText::Format(LOCTEXT("AuditSummary", "Asset Cleaner: {0} orphan candidate package(s). See Output Log. No assets were modified."), FText::AsNumber(Result.Graph.OrphanPackages.Num())) : LOCTEXT("AuditIncomplete", "Asset Cleaner: Graph analysis is incomplete. No cleanup candidates were produced."), Result.Graph.bComplete ? SNotificationItem::CS_Success : SNotificationItem::CS_None);
}

void FBertaAssetCleaner::CleanUnusedAssets(const TArray<FAssetData>& Assets)
{
	FInspectionResult Result;
	if (!InspectAssets(Assets, TEXT("Clean preflight"), Result)) return;
	LogResults(Result, TEXT("Clean preflight"));
	TArray<FAssetData> Candidates;
	CollectCandidateAssets(Result, Candidates);
	if (Candidates.IsEmpty())
	{
		ShowAssetCleanerNotification(LOCTEXT("NoOrphans", "Asset Cleaner: No current orphan candidates to clean."), SNotificationItem::CS_None);
		return;
	}
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	TSet<FName> CandidateObjectPaths;
	TSet<FName> CandidatePackages;
	for (const FAssetData& Candidate : Candidates)
	{
		CandidateObjectPaths.Add(*Candidate.GetObjectPathString());
		CandidatePackages.Add(Candidate.PackageName);
	}
	TSet<FName> ChangedPackages;
	for (const FName PackageName : CandidatePackages)
	{
		TArray<FAssetData> CurrentPackageAssets;
		if (!Registry.GetAssetsByPackageName(PackageName, CurrentPackageAssets))
		{
			ChangedPackages.Add(PackageName);
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - package could not be revalidated before deletion."), *PackageName.ToString());
			continue;
		}
		for (const FAssetData& CurrentAsset : CurrentPackageAssets)
		{
			if (!CandidateObjectPaths.Contains(*CurrentAsset.GetObjectPathString()))
			{
				ChangedPackages.Add(PackageName);
				UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - package now contains an additional asset outside the current orphan analysis."), *PackageName.ToString());
				break;
			}
		}
	}
	Candidates.RemoveAll([&ChangedPackages](const FAssetData& Candidate) { return ChangedPackages.Contains(Candidate.PackageName); });
	if (Candidates.IsEmpty())
	{
		ShowAssetCleanerNotification(LOCTEXT("NoRevalidatedOrphans", "Asset Cleaner: No current orphan candidates could be safely revalidated for deletion."), SNotificationItem::CS_None);
		return;
	}
	TArray<UObject*> Loaded;
	const AssetViewUtils::FLoadAssetsSettings Settings{ .bFollowRedirectors = false, .bAllowCancel = true };
	if (AssetViewUtils::LoadAssetsIfNeeded(Candidates, Loaded, Settings) == AssetViewUtils::ELoadAssetsResult::Cancelled) return;
	TArray<UObject*> LoadedCandidates;
	CollectLoadedCandidateObjects(Candidates, Loaded, LoadedCandidates);
	TSet<FName> LoadedPaths;
	for (const UObject* Object : LoadedCandidates) LoadedPaths.Add(*Object->GetPathName());
	TSet<FName> FailedPackages;
	for (const FAssetData& Candidate : Candidates)
	{
		if (!LoadedPaths.Contains(*Candidate.GetObjectPathString()))
		{
			FailedPackages.Add(Candidate.PackageName);
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - a package asset failed to load for native deletion."), *Candidate.GetObjectPathString());
		}
	}
	LoadedCandidates.RemoveAll([&FailedPackages](const UObject* Object) { return FailedPackages.Contains(Object->GetOutermost()->GetFName()); });
	if (LoadedCandidates.IsEmpty())
	{
		ShowAssetCleanerNotification(LOCTEXT("NoLoadedOrphans", "Asset Cleaner: No current orphan candidates could be loaded for deletion."), SNotificationItem::CS_None);
		return;
	}
	if (Registry.IsGathering())
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] Clean aborted before native deletion because the Asset Registry began gathering."));
		ShowAssetCleanerNotification(LOCTEXT("RegistryGatheringBeforeDelete", "Asset Cleaner cannot clean while the Asset Registry is gathering. Try again when scanning completes."), SNotificationItem::CS_None);
		return;
	}
	const int32 Deleted = AssetViewUtils::DeleteAssets(LoadedCandidates);
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] Clean complete: %d of %d candidate asset(s) deleted by Unreal's native deletion workflow."), Deleted, LoadedCandidates.Num());
	ShowAssetCleanerNotification(FText::Format(LOCTEXT("CleanSummary", "Asset Cleaner: Unreal deleted {0} of {1} current orphan candidate asset(s). See Output Log."), FText::AsNumber(Deleted), FText::AsNumber(LoadedCandidates.Num())), Deleted > 0 ? SNotificationItem::CS_Success : SNotificationItem::CS_None);
}

bool FBertaAssetCleaner::IsPathWithinSelectedFolderScopes(const FString& FolderPath, const TArray<FString>& SelectedFolderScopes)
{
	const FString NormalizedFolderPath = NormalizeFolderPath(FolderPath);
	return SelectedFolderScopes.ContainsByPredicate([&NormalizedFolderPath](const FString& Scope)
	{
		const FString NormalizedScope = NormalizeFolderPath(Scope);
		return NormalizedFolderPath == NormalizedScope || NormalizedFolderPath.StartsWith(NormalizedScope + TEXT("/"));
	});
}

bool FBertaAssetCleaner::IsProtectedEmptyFolderPath(const FString& FolderPath)
{
	const FString NormalizedFolderPath = NormalizeFolderPath(FolderPath);
	return NormalizedFolderPath == TEXT("/Game")
		|| ContainsPackagePathSegment(NormalizedFolderPath, FPackagePath::GetExternalActorsFolderName())
		|| ContainsPackagePathSegment(NormalizedFolderPath, FPackagePath::GetExternalObjectsFolderName())
		|| ContainsPackagePathSegment(NormalizedFolderPath, TEXT("EDL"))
		|| FExternalDataLayerHelper::IsExternalDataLayerPath(NormalizedFolderPath);
}

void FBertaAssetCleaner::CollapseEmptyFolderCandidates(const TArray<FString>& CandidatePaths, const TArray<FString>& SelectedFolderScopes, TArray<FString>& OutDeleteRoots)
{
	OutDeleteRoots.Reset();
	TSet<FString> UniqueCandidates;
	for (const FString& CandidatePath : CandidatePaths)
	{
		const FString NormalizedCandidatePath = NormalizeFolderPath(CandidatePath);
		if (IsProjectFolderPath(NormalizedCandidatePath)
			&& !IsProtectedEmptyFolderPath(NormalizedCandidatePath)
			&& IsPathWithinSelectedFolderScopes(NormalizedCandidatePath, SelectedFolderScopes))
		{
			UniqueCandidates.Add(NormalizedCandidatePath);
		}
	}

	TArray<FString> OrderedCandidates = UniqueCandidates.Array();
	OrderedCandidates.Sort([](const FString& Left, const FString& Right)
	{
		return Left.Len() == Right.Len() ? Left < Right : Left.Len() < Right.Len();
	});
	for (const FString& CandidatePath : OrderedCandidates)
	{
		if (!OutDeleteRoots.ContainsByPredicate([&CandidatePath](const FString& AcceptedPath)
		{
			return CandidatePath == AcceptedPath || CandidatePath.StartsWith(AcceptedPath + TEXT("/"));
		}))
		{
			OutDeleteRoots.Add(CandidatePath);
		}
	}
}

void FBertaAssetCleaner::CleanEmptyFolders(const TArray<FString>& SelectedFolders)
{
	TArray<FString> SelectedFolderScopes;
	for (const FString& SelectedFolder : SelectedFolders)
	{
		const FString NormalizedFolderPath = NormalizeFolderPath(SelectedFolder);
		FString Directory;
		if (!TryGetProjectContentDirectory(NormalizedFolderPath, Directory))
		{
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - selected folder could not be mapped safely to project Content."), *SelectedFolder);
			continue;
		}
		SelectedFolderScopes.AddUnique(NormalizedFolderPath);
	}

	if (SelectedFolderScopes.IsEmpty())
	{
		ShowAssetCleanerNotification(LOCTEXT("NoValidFolderScope", "Asset Cleaner: No valid project Content folder scope was selected."), SNotificationItem::CS_None);
		return;
	}

	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	if (Registry.IsGathering())
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] Empty folder cleanup aborted because the Asset Registry is gathering."));
		ShowAssetCleanerNotification(LOCTEXT("RegistryGatheringEmptyFolders", "Asset Cleaner cannot clean empty folders while the Asset Registry is gathering. Try again when scanning completes."), SNotificationItem::CS_None);
		return;
	}

	TSet<FString> DiscoveredFolderPaths;
	int32 DirectoriesInspected = 0;
	int32 SkippedDirectories = 0;
	for (const FString& Scope : SelectedFolderScopes)
	{
		FString Directory;
		if (TryGetProjectContentDirectory(Scope, Directory) && FPaths::DirectoryExists(Directory))
		{
			AddFolderAndDescendants(Scope, Directory, DiscoveredFolderPaths, DirectoriesInspected, SkippedDirectories);
		}
		else
		{
			++SkippedDirectories;
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - selected physical directory does not exist."), *Scope);
		}
	}

	TArray<FString> EmptyDirectories;
	for (const FString& FolderPath : DiscoveredFolderPaths)
	{
		if (!IsPathWithinSelectedFolderScopes(FolderPath, SelectedFolderScopes) || IsProtectedEmptyFolderPath(FolderPath))
		{
			++SkippedDirectories;
			continue;
		}
		if (Registry.HasAssets(*FolderPath, true))
		{
			continue;
		}

		FString Directory;
		bool bEnumerated = false;
		if (!TryGetProjectContentDirectory(FolderPath, Directory) || DoesDirectoryTreeContainFiles(Directory, bEnumerated) || !bEnumerated)
		{
			if (!bEnumerated)
			{
				++SkippedDirectories;
				UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - physical subtree could not be inspected."), *FolderPath);
			}
			continue;
		}
		EmptyDirectories.Add(FolderPath);
	}

	TArray<FString> DeleteRoots;
	CollapseEmptyFolderCandidates(EmptyDirectories, SelectedFolderScopes, DeleteRoots);
	for (const FString& DeleteRoot : DeleteRoots)
	{
		UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] EMPTY FOLDER CANDIDATE: %s"), *DeleteRoot);
	}
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] Empty folder preflight: %d selected root(s), %d directory/directories inspected, %d empty directory/directories discovered, %d collapsed delete root(s), %d protected/skipped directory/directories."), SelectedFolderScopes.Num(), DirectoriesInspected, EmptyDirectories.Num(), DeleteRoots.Num(), SkippedDirectories);
	if (DeleteRoots.IsEmpty())
	{
		ShowAssetCleanerNotification(LOCTEXT("NoEmptyFolders", "Asset Cleaner: No empty folders found in the selected scope."), SNotificationItem::CS_Success);
		return;
	}

	const FText ConfirmationText = FText::Format(LOCTEXT("ConfirmEmptyFolderDelete", "Delete {0} empty project Content folder(s)?\n\nThese folders were found to contain no Asset Registry assets and no files. They will be revalidated immediately before deletion.\n\nSee Output Log for the candidate paths."), FText::AsNumber(DeleteRoots.Num()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ConfirmationText, LOCTEXT("ConfirmEmptyFolderDeleteTitle", "Clean Empty Folders")) != EAppReturnType::Yes)
	{
		return;
	}

	if (Registry.IsGathering())
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] Empty folder cleanup aborted before deletion because the Asset Registry began gathering."));
		ShowAssetCleanerNotification(LOCTEXT("RegistryGatheringBeforeEmptyFolderDelete", "Asset Cleaner cannot clean empty folders while the Asset Registry is gathering. Try again when scanning completes."), SNotificationItem::CS_None);
		return;
	}

	TArray<FString> RevalidatedDeleteRoots;
	int32 RevalidationSkips = 0;
	for (const FString& DeleteRoot : DeleteRoots)
	{
		FString Directory;
		bool bEnumerated = false;
		const bool bStillSafe = IsPathWithinSelectedFolderScopes(DeleteRoot, SelectedFolderScopes)
			&& !IsProtectedEmptyFolderPath(DeleteRoot)
			&& TryGetProjectContentDirectory(DeleteRoot, Directory)
			&& FPaths::DirectoryExists(Directory)
			&& !Registry.HasAssets(*DeleteRoot, true)
			&& !DoesDirectoryTreeContainFiles(Directory, bEnumerated)
			&& bEnumerated;
		if (bStillSafe)
		{
			RevalidatedDeleteRoots.Add(DeleteRoot);
		}
		else
		{
			++RevalidationSkips;
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] SKIPPED: %s - folder changed after preflight or could not be safely revalidated."), *DeleteRoot);
		}
	}
	if (RevalidatedDeleteRoots.IsEmpty())
	{
		ShowAssetCleanerNotification(LOCTEXT("NoRevalidatedEmptyFolders", "Asset Cleaner: No empty folders could be safely revalidated for deletion."), SNotificationItem::CS_None);
		return;
	}
	if (Registry.IsGathering())
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] Empty folder cleanup aborted because the Asset Registry began gathering during revalidation."));
		ShowAssetCleanerNotification(LOCTEXT("RegistryGatheringDuringEmptyFolderRevalidation", "Asset Cleaner cannot clean empty folders while the Asset Registry is gathering. Try again when scanning completes."), SNotificationItem::CS_None);
		return;
	}

	AssetViewUtils::FDeleteFolderParameters DeleteParameters;
	DeleteParameters.PathsToDelete = RevalidatedDeleteRoots;
	DeleteParameters.bShowConfirmationBeforeLoadingAssets = false;
	const bool bNativeDeletionReportedSuccess = AssetViewUtils::DeleteFolders(DeleteParameters);

	int32 Deleted = 0;
	int32 Failed = 0;
	for (const FString& DeleteRoot : RevalidatedDeleteRoots)
	{
		FString Directory;
		if (TryGetProjectContentDirectory(DeleteRoot, Directory) && !FPaths::DirectoryExists(Directory))
		{
			++Deleted;
		}
		else
		{
			++Failed;
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetCleaner] FAILED: %s - folder remains after native deletion."), *DeleteRoot);
		}
	}
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("[AssetCleaner] Empty folder cleanup complete: %d deleted, %d remaining, %d skipped during revalidation, %d failed. Native workflow result: %s."), Deleted, Failed, RevalidationSkips, Failed, bNativeDeletionReportedSuccess ? TEXT("success") : TEXT("failure"));
	ShowAssetCleanerNotification(FText::Format(LOCTEXT("CleanEmptyFoldersSummary", "Asset Cleaner: {0} empty folder(s) deleted, {1} skipped, {2} failed. See Output Log."), FText::AsNumber(Deleted), FText::AsNumber(RevalidationSkips), FText::AsNumber(Failed)), Failed == 0 ? SNotificationItem::CS_Success : SNotificationItem::CS_None);
}

void FBertaAssetCleaner::CollectLoadedCandidateObjects(const TArray<FAssetData>& Candidates, TConstArrayView<UObject*> LoadedObjects, TArray<UObject*>& OutLoadedCandidates)
{
	OutLoadedCandidates.Reset();
	TSet<FName> CandidatePaths;
	for (const FAssetData& Candidate : Candidates) CandidatePaths.Add(*Candidate.GetObjectPathString());
	for (UObject* Object : LoadedObjects) if (Object && CandidatePaths.Contains(*Object->GetPathName()) && !OutLoadedCandidates.Contains(Object)) OutLoadedCandidates.Add(Object);
}

#undef LOCTEXT_NAMESPACE
