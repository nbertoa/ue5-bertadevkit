#include "AssetActions/BertaAssetCleaner.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "UObject/ObjectRedirector.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	FAssetData MakeCleanerAssetData(const TCHAR* PackageName, const TCHAR* AssetName, const FTopLevelAssetPath& ClassPath)
	{
		const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);
		return FAssetData(FName(PackageName), FName(*PackagePath), FName(AssetName), ClassPath);
	}

	FBertaAssetCleanerInspection MakeInspection(int32 ReferencerCount = 0)
	{
		FBertaAssetCleanerInspection Inspection;
		Inspection.bReferencerQuerySucceeded = true;
		Inspection.ReferencerCount = ReferencerCount;
		Inspection.bPrimaryAssetQueryAvailable = true;
		return Inspection;
	}

	void TestClassification(FAutomationTestBase& Test, const TCHAR* Description, const FAssetData& AssetData, const FBertaAssetCleanerInspection& Inspection, EBertaAssetCleanerClassification Expected)
	{
		Test.TestEqual(Description, FBertaAssetCleaner::ClassifyAsset(AssetData, Inspection).Classification, Expected);
	}

	FBertaAssetCleanerAssetResult MakeInspectionResult(const FAssetData& AssetData, EBertaAssetCleanerClassification Classification)
	{
		FBertaAssetCleanerAssetResult Result;
		Result.AssetData = AssetData;
		Result.Classification.Classification = Classification;
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetCleanerClassificationTest, "BertaDevKit.AssetCleaner.Classification", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetCleanerClassificationTest::RunTest(const FString& Parameters)
{
	const FAssetData Ordinary = MakeCleanerAssetData(TEXT("/Game/CleanerTests/T_Ordinary"), TEXT("T_Ordinary"), UStaticMesh::StaticClass()->GetClassPathName());
	TestClassification(*this, TEXT("A global referencer outside the audit scope keeps an ordinary asset referenced"), Ordinary, MakeInspection(1), EBertaAssetCleanerClassification::Referenced);
	TestClassification(*this, TEXT("An ordinary asset with no referencers is an unused candidate"), Ordinary, MakeInspection(), EBertaAssetCleanerClassification::UnusedCandidate);

	const FAssetData World = MakeCleanerAssetData(TEXT("/Game/CleanerTests/TestMap"), TEXT("TestMap"), UWorld::StaticClass()->GetClassPathName());
	TestClassification(*this, TEXT("An unreferenced world is protected"), World, MakeInspection(), EBertaAssetCleanerClassification::Protected);

	const FAssetData Redirector = MakeCleanerAssetData(TEXT("/Game/CleanerTests/OldAsset"), TEXT("OldAsset"), UObjectRedirector::StaticClass()->GetClassPathName());
	TestClassification(*this, TEXT("An unreferenced redirector is protected"), Redirector, MakeInspection(), EBertaAssetCleanerClassification::Protected);

	FBertaAssetCleanerInspection PrimaryInspection = MakeInspection();
	PrimaryInspection.bIsRegisteredPrimaryAsset = true;
	TestClassification(*this, TEXT("A registered Primary Asset is protected"), Ordinary, PrimaryInspection, EBertaAssetCleanerClassification::Protected);

	const FAssetData ExternalActor = MakeCleanerAssetData(TEXT("/Game/__ExternalActors__/TestMap/A/B/Actor"), TEXT("Actor"), UStaticMesh::StaticClass()->GetClassPathName());
	TestClassification(*this, TEXT("World Partition external actor storage is protected"), ExternalActor, MakeInspection(), EBertaAssetCleanerClassification::Protected);

	const FAssetData ExternalObject = MakeCleanerAssetData(TEXT("/Game/__ExternalObjects__/TestMap/A/B/Object"), TEXT("Object"), UStaticMesh::StaticClass()->GetClassPathName());
	TestClassification(*this, TEXT("External object storage is protected"), ExternalObject, MakeInspection(), EBertaAssetCleanerClassification::Protected);

	const FAssetData ExternalDataLayer = MakeCleanerAssetData(TEXT("/Game/EDL/1A2B3C4D/TestMap/Asset"), TEXT("Asset"), UStaticMesh::StaticClass()->GetClassPathName());
	TestClassification(*this, TEXT("External Data Layer storage is protected"), ExternalDataLayer, MakeInspection(), EBertaAssetCleanerClassification::Protected);

	FBertaAssetCleanerInspection FailedQuery = MakeInspection();
	FailedQuery.bReferencerQuerySucceeded = false;
	TestClassification(*this, TEXT("A failed referencer query never produces an unused candidate"), Ordinary, FailedQuery, EBertaAssetCleanerClassification::Skipped);

	FBertaAssetCleanerInspection MissingAssetManager = MakeInspection();
	MissingAssetManager.bPrimaryAssetQueryAvailable = false;
	TestClassification(*this, TEXT("An unavailable Asset Manager never produces an unused candidate"), Ordinary, MissingAssetManager, EBertaAssetCleanerClassification::Skipped);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetCleanerCleanPreflightTest, "BertaDevKit.AssetCleaner.CleanPreflight", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetCleanerCleanPreflightTest::RunTest(const FString& Parameters)
{
	const FAssetData Candidate = MakeCleanerAssetData(TEXT("/Game/CleanerTests/T_Candidate"), TEXT("T_Candidate"), UStaticMesh::StaticClass()->GetClassPathName());
	const FAssetData Referenced = MakeCleanerAssetData(TEXT("/Game/CleanerTests/T_Referenced"), TEXT("T_Referenced"), UStaticMesh::StaticClass()->GetClassPathName());
	const FAssetData Protected = MakeCleanerAssetData(TEXT("/Game/CleanerTests/TestMap"), TEXT("TestMap"), UWorld::StaticClass()->GetClassPathName());
	const FAssetData Skipped = MakeCleanerAssetData(TEXT("/Game/CleanerTests/T_Skipped"), TEXT("T_Skipped"), UStaticMesh::StaticClass()->GetClassPathName());
	const FAssetData OutsideScope = MakeCleanerAssetData(TEXT("/Game/OtherFolder/T_Unrelated"), TEXT("T_Unrelated"), UStaticMesh::StaticClass()->GetClassPathName());

	TArray<FBertaAssetCleanerAssetResult> CurrentInspectionResults;
	CurrentInspectionResults.Add(MakeInspectionResult(Candidate, EBertaAssetCleanerClassification::UnusedCandidate));
	CurrentInspectionResults.Add(MakeInspectionResult(Referenced, EBertaAssetCleanerClassification::Referenced));
	CurrentInspectionResults.Add(MakeInspectionResult(Protected, EBertaAssetCleanerClassification::Protected));
	CurrentInspectionResults.Add(MakeInspectionResult(Skipped, EBertaAssetCleanerClassification::Skipped));

	TArray<FAssetData> DeleteCandidates;
	FBertaAssetCleaner::CollectUnusedCandidateAssets(CurrentInspectionResults, DeleteCandidates);
	TestEqual(TEXT("Only current unused candidates enter the delete candidate set"), DeleteCandidates.Num(), 1);
	if (DeleteCandidates.Num() == 1)
	{
		TestEqual(TEXT("The unused candidate is retained"), DeleteCandidates[0].GetObjectPathString(), Candidate.GetObjectPathString());
	}
	TestFalse(TEXT("Referenced assets never enter the delete candidate set"), DeleteCandidates.ContainsByPredicate([&Referenced](const FAssetData& Asset) { return Asset.GetObjectPathString() == Referenced.GetObjectPathString(); }));
	TestFalse(TEXT("Protected assets never enter the delete candidate set"), DeleteCandidates.ContainsByPredicate([&Protected](const FAssetData& Asset) { return Asset.GetObjectPathString() == Protected.GetObjectPathString(); }));
	TestFalse(TEXT("Skipped assets never enter the delete candidate set"), DeleteCandidates.ContainsByPredicate([&Skipped](const FAssetData& Asset) { return Asset.GetObjectPathString() == Skipped.GetObjectPathString(); }));
	TestFalse(TEXT("Assets outside the supplied inspection scope never enter the delete candidate set"), DeleteCandidates.ContainsByPredicate([&OutsideScope](const FAssetData& Asset) { return Asset.GetObjectPathString() == OutsideScope.GetObjectPathString(); }));

	TArray<FBertaAssetCleanerAssetResult> ReclassifiedResults;
	ReclassifiedResults.Add(MakeInspectionResult(Candidate, EBertaAssetCleanerClassification::Referenced));
	FBertaAssetCleaner::CollectUnusedCandidateAssets(ReclassifiedResults, DeleteCandidates);
	TestEqual(TEXT("The current reclassification is used instead of a prior audit candidate state"), DeleteCandidates.Num(), 0);

	TArray<UObject*> LoadedDeleteCandidates;
	const TArray<FAssetData> SingleCandidate = { Candidate };
	const TArray<UObject*> NoLoadedObjects;
	FBertaAssetCleaner::CollectLoadedCandidateObjects(SingleCandidate, NoLoadedObjects, LoadedDeleteCandidates);
	TestEqual(TEXT("A candidate that failed to load cannot enter the native deletion workflow"), LoadedDeleteCandidates.Num(), 0);

	UPackage* LoadedCandidatePackage = CreatePackage(TEXT("/Game/CleanerTests/LoadedCandidate"));
	UObject* LoadedCandidate = NewObject<UStaticMesh>(LoadedCandidatePackage, TEXT("LoadedCandidate"), RF_Transient);
	const FAssetData LoadedCandidateData(LoadedCandidate);
	const TArray<FAssetData> LoadedCandidateArray = { LoadedCandidateData };
	const TArray<UObject*> MixedLoadedObjects = { LoadedCandidate, NewObject<UStaticMesh>(GetTransientPackage(), TEXT("UnrelatedLoadedObject"), RF_Transient) };
	FBertaAssetCleaner::CollectLoadedCandidateObjects(LoadedCandidateArray, MixedLoadedObjects, LoadedDeleteCandidates);
	TestEqual(TEXT("Only loaded objects that match current candidates enter the native deletion workflow"), LoadedDeleteCandidates.Num(), 1);
	if (LoadedDeleteCandidates.Num() == 1)
	{
		TestEqual(TEXT("The matching loaded candidate is retained"), LoadedDeleteCandidates[0], LoadedCandidate);
	}

	return true;
}

#endif
