#include "AssetActions/BertaAssetCleaner.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "UObject/ObjectRedirector.h"

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

#endif
