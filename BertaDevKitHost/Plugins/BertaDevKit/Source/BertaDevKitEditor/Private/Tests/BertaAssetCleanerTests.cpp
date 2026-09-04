#include "AssetActions/BertaAssetCleaner.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "UObject/ObjectRedirector.h"

namespace
{
	FBertaAssetCleanerPackageRecord MakePackage(const TCHAR* Name, bool bProtected = false, bool bSkipped = false, bool bExternal = false)
	{
		FBertaAssetCleanerPackageRecord Result;
		Result.PackageName = FName(Name);
		Result.bProtected = bProtected;
		Result.bSkipped = bSkipped;
		Result.bHasExternalReferencer = bExternal;
		return Result;
	}

	FAssetData MakeAssetData(const TCHAR* PackageName, const TCHAR* AssetName, const FTopLevelAssetPath& ClassPath)
	{
		const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);
		return FAssetData(FName(PackageName), FName(*PackagePath), FName(AssetName), ClassPath);
	}

	void AddDependency(TArray<FBertaAssetCleanerPackageRecord>& Records, const TCHAR* From, const TCHAR* To)
	{
		for (FBertaAssetCleanerPackageRecord& Record : Records)
		{
			if (Record.PackageName == FName(From)) { Record.DependencyPackages.Add(FName(To)); return; }
		}
	}

	void TestContains(FAutomationTestBase& Test, const TCHAR* Description, const TSet<FName>& Set, const TCHAR* Name, bool bExpected)
	{
		Test.TestEqual(Description, Set.Contains(FName(Name)), bExpected);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetCleanerClassificationTest, "BertaDevKit.AssetCleaner.Classification", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetCleanerClassificationTest::RunTest(const FString& Parameters)
{
	FBertaAssetCleanerInspection Inspection;
	Inspection.bReferencerQuerySucceeded = true;
	Inspection.bPrimaryAssetQueryAvailable = true;
	FAssetData World = MakeAssetData(TEXT("/Game/TestMap"), TEXT("TestMap"), UWorld::StaticClass()->GetClassPathName());
	FAssetData Redirector = MakeAssetData(TEXT("/Game/Old"), TEXT("Old"), UObjectRedirector::StaticClass()->GetClassPathName());
	TestEqual(TEXT("World is protected"), FBertaAssetCleaner::ClassifyAsset(World, Inspection).Classification, EBertaAssetCleanerClassification::Protected);
	TestEqual(TEXT("Redirector is protected"), FBertaAssetCleaner::ClassifyAsset(Redirector, Inspection).Classification, EBertaAssetCleanerClassification::Protected);
	Inspection.bReferencerQuerySucceeded = false;
	TestEqual(TEXT("Failed referencer query is skipped"), FBertaAssetCleaner::ClassifyAsset(World, Inspection).Classification, EBertaAssetCleanerClassification::Skipped);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetCleanerGraphTest, "BertaDevKit.AssetCleaner.Graph", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetCleanerGraphTest::RunTest(const FString& Parameters)
{
	{ TArray Records{ MakePackage(TEXT("/Game/A")) }; const auto R = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestContains(*this, TEXT("Direct orphan"), R.OrphanPackages, TEXT("/Game/A"), true); }
	{ TArray Records{ MakePackage(TEXT("/Game/A"), false, false, true), MakePackage(TEXT("/Game/B")), MakePackage(TEXT("/Game/C")) }; AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/B")); AddDependency(Records, TEXT("/Game/B"), TEXT("/Game/C")); const auto R = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestEqual(TEXT("External live chain has no orphans"), R.OrphanPackages.Num(), 0); }
	{ TArray Records{ MakePackage(TEXT("/Game/A")), MakePackage(TEXT("/Game/B")), MakePackage(TEXT("/Game/C")) }; AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/B")); AddDependency(Records, TEXT("/Game/B"), TEXT("/Game/C")); const auto R = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestEqual(TEXT("Orphan chain has all packages"), R.OrphanPackages.Num(), 3); TestEqual(TEXT("Orphan chain is one group"), R.OrphanGroups.Num(), 1); }
	{ TArray Records{ MakePackage(TEXT("/Game/A")), MakePackage(TEXT("/Game/B")), MakePackage(TEXT("/Game/C")), MakePackage(TEXT("/Game/D")) }; AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/B")); AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/C")); AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/D")); const auto R = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestEqual(TEXT("Branching orphan is one group"), R.OrphanGroups.Num(), 1); TestEqual(TEXT("Branching group has four packages"), R.OrphanGroups[0].Num(), 4); }
	{ TArray Records{ MakePackage(TEXT("/Game/A")), MakePackage(TEXT("/Game/B")), MakePackage(TEXT("/Game/C")) }; AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/B")); AddDependency(Records, TEXT("/Game/B"), TEXT("/Game/C")); AddDependency(Records, TEXT("/Game/C"), TEXT("/Game/A")); const auto R = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestEqual(TEXT("Cycle is orphaned"), R.OrphanPackages.Num(), 3); Records[1].bHasExternalReferencer = true; const auto Anchored = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestEqual(TEXT("Anchored cycle is live"), Anchored.OrphanPackages.Num(), 0); }
	{ TArray Records{ MakePackage(TEXT("/Game/Map"), true), MakePackage(TEXT("/Game/A")), MakePackage(TEXT("/Game/B")) }; AddDependency(Records, TEXT("/Game/Map"), TEXT("/Game/A")); AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/B")); const auto R = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestEqual(TEXT("Protected root keeps dependencies live"), R.OrphanPackages.Num(), 0); }
	{ TArray Records{ MakePackage(TEXT("/Game/A")), MakePackage(TEXT("/Game/B")) }; AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/B")); AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/Outside")); const auto R = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestEqual(TEXT("Outgoing external dependency does not anchor source"), R.OrphanPackages.Num(), 2); Records[0].bDependencyQuerySucceeded = false; const auto Failed = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestFalse(TEXT("Failed graph query is incomplete"), Failed.bComplete); TestEqual(TEXT("Incomplete graph has no deletion candidates"), Failed.OrphanPackages.Num(), 0); }
	{ TArray Records{ MakePackage(TEXT("/Game/A")), MakePackage(TEXT("/Game/B"), false, true) }; AddDependency(Records, TEXT("/Game/A"), TEXT("/Game/A")); const auto R = FBertaAssetCleaner::AnalyzePackageGraph(Records); TestContains(*this, TEXT("Self reference remains orphan"), R.OrphanPackages, TEXT("/Game/A"), true); TestContains(*this, TEXT("Skipped package never orphan"), R.OrphanPackages, TEXT("/Game/B"), false); }
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetCleanerEmptyFolderPathsTest, "BertaDevKit.AssetCleaner.EmptyFolderPaths", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetCleanerEmptyFolderPathsTest::RunTest(const FString& Parameters)
{
	TArray<FString> Collapsed;
	FBertaAssetCleaner::CollapseEmptyFolderCandidates({ TEXT("/Game/A"), TEXT("/Game/A/B"), TEXT("/Game/A/B/C"), TEXT("/Game/X"), TEXT("/Game/X/Y") }, { TEXT("/Game") }, Collapsed);
	TestEqual(TEXT("Nested empty folders collapse to two roots"), Collapsed.Num(), 2);
	TestTrue(TEXT("A is retained as the highest root"), Collapsed.Contains(TEXT("/Game/A")));
	TestTrue(TEXT("X is retained as the highest root"), Collapsed.Contains(TEXT("/Game/X")));

	FBertaAssetCleaner::CollapseEmptyFolderCandidates({ TEXT("/Game/A"), TEXT("/Game/A/B"), TEXT("/Game/A/B/C") }, { TEXT("/Game/A/B") }, Collapsed);
	TestEqual(TEXT("Selected scope does not broaden upward"), Collapsed.Num(), 1);
	TestTrue(TEXT("Selected nested scope is retained"), Collapsed.Contains(TEXT("/Game/A/B")));
	TestTrue(TEXT("Game root is never an empty-folder candidate"), FBertaAssetCleaner::IsProtectedEmptyFolderPath(TEXT("/Game")));
	TestTrue(TEXT("External actors are protected"), FBertaAssetCleaner::IsProtectedEmptyFolderPath(TEXT("/Game/Maps/__ExternalActors__/Level")));
	TestTrue(TEXT("External objects are protected"), FBertaAssetCleaner::IsProtectedEmptyFolderPath(TEXT("/Game/Maps/__ExternalObjects__/Level")));
	FBertaAssetCleaner::CollapseEmptyFolderCandidates({ TEXT("/Game/D"), TEXT("/Game/D"), TEXT("/Game/D/E") }, { TEXT("/Game/D"), TEXT("/Game/D/E") }, Collapsed);
	TestEqual(TEXT("Overlapping candidates are deduplicated"), Collapsed.Num(), 1);
	TestTrue(TEXT("Duplicate collapse root is retained"), Collapsed.Contains(TEXT("/Game/D")));
	return true;
}

#endif
