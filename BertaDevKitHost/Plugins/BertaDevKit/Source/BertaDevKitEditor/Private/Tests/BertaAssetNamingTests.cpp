#include "AssetActions/BertaAssetNamingUtils.h"
#include "AssetActions/BertaAssetNamingBatch.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimBlueprint.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/BlueprintSupport.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"

namespace
{
	FAssetData MakeAssetData(FName Name, UClass* Class, const FString& NativeParentPath = FString())
	{
		FAssetDataTagMap Tags;
		if (!NativeParentPath.IsEmpty())
		{
			Tags.Add(FBlueprintTags::NativeParentClassPath, NativeParentPath);
		}

		return FAssetData(FName(TEXT("/Game/AssetNamingTests")), FName(TEXT("/Game")), Name, Class->GetClassPathName(), MoveTemp(Tags));
	}

	FAssetData MakeAssetData(FName Name, const FTopLevelAssetPath& ClassPath)
	{
		return FAssetData(FName(TEXT("/Game/AssetNamingTests")), FName(TEXT("/Game")), Name, ClassPath);
	}

	void TestPlan(FAutomationTestBase& Test, const FAssetData& Asset, EBertaAssetNamingStatus Status, const TCHAR* Prefix, const TCHAR* Target)
	{
		const FBertaAssetNamingPlan Plan = UBertaAssetNamingUtils::BuildRenamePlan(Asset);
		Test.TestEqual(TEXT("Status"), Plan.Status, Status);
		Test.TestEqual(TEXT("Prefix"), Plan.ExpectedPrefix, FString(Prefix));
		Test.TestEqual(TEXT("Target"), Plan.TargetName, FString(Target));
	}

	FBertaAssetNamingBatchCandidate MakeBatchCandidate(FAutomationTestBase& Test, const TCHAR* PackageName, const TCHAR* AssetName, const TCHAR* TargetName)
	{
		const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);
		const FAssetData AssetData(FName(PackageName), FName(*PackagePath), FName(AssetName), UStaticMesh::StaticClass()->GetClassPathName());
		FBertaAssetNamingPlan Plan;
		Plan.Status = EBertaAssetNamingStatus::NeedsRename;
		Plan.ExpectedPrefix = TEXT("SM_");
		Plan.TargetName = TargetName;

		FBertaAssetNamingBatchCandidate Candidate;
		FText FailureReason;
		Test.TestTrue(FString::Printf(TEXT("Build batch candidate %s"), PackageName), BertaAssetNamingBatch::BuildCandidate(AssetData, Plan, Candidate, FailureReason));
		return Candidate;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetNamingCoreTest, "BertaDevKit.AssetNaming.Core", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetNamingCoreTest::RunTest(const FString& Parameters)
{
	TestPlan(*this, MakeAssetData(TEXT("Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("SM_Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::AlreadyCorrect, TEXT("SM_"), TEXT("SM_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("Actor"), UBlueprint::StaticClass(), TEXT("Class'/Script/Engine.Actor'")), EBertaAssetNamingStatus::NeedsRename, TEXT("BP_"), TEXT("BP_Actor"));
	TestPlan(*this, MakeAssetData(TEXT("Character"), UBlueprint::StaticClass(), TEXT("Class'/Script/Engine.Character'")), EBertaAssetNamingStatus::NeedsRename, TEXT("CH_"), TEXT("CH_Character"));
	TestPlan(*this, MakeAssetData(TEXT("ChildCharacter"), UBlueprint::StaticClass(), TEXT("Class'/Script/Engine.Character'")), EBertaAssetNamingStatus::NeedsRename, TEXT("CH_"), TEXT("CH_ChildCharacter"));
	TestPlan(*this, MakeAssetData(TEXT("Widget"), UBlueprint::StaticClass(), TEXT("Class'/Script/UMG.UserWidget'")), EBertaAssetNamingStatus::NeedsRename, TEXT("WBP_"), TEXT("WBP_Widget"));
	TestPlan(*this, MakeAssetData(TEXT("Anim"), UAnimBlueprint::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("ABP_"), TEXT("ABP_Anim"));
	TestPlan(*this, MakeAssetData(TEXT("Ability"), UBlueprint::StaticClass(), TEXT("Class'/Script/GameplayAbilities.GameplayAbility'")), EBertaAssetNamingStatus::NeedsRename, TEXT("GA_"), TEXT("GA_Ability"));
	TestPlan(*this, MakeAssetData(TEXT("AbilityBlueprint"), FTopLevelAssetPath(TEXT("/Script/GameplayAbilities"), TEXT("GameplayAbilityBlueprint"))), EBertaAssetNamingStatus::NeedsRename, TEXT("GA_"), TEXT("GA_AbilityBlueprint"));
	TestPlan(*this, MakeAssetData(TEXT("M_Foo_Inst"), UMaterialInstanceConstant::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("MI_"), TEXT("MI_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("MI_Foo"), UMaterialInstanceConstant::StaticClass()), EBertaAssetNamingStatus::AlreadyCorrect, TEXT("MI_"), TEXT("MI_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("Foo_Montage"), UAnimMontage::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("AM_"), TEXT("AM_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("Unknown"), UObject::StaticClass()), EBertaAssetNamingStatus::UnknownClass, TEXT(""), TEXT(""));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetNamingBatchTest, "BertaDevKit.AssetNaming.Batch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetNamingBatchTest::RunTest(const FString& Parameters)
{
	{
		TArray<FBertaAssetNamingBatchCandidate> Candidates;
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Distinct/Foo"), TEXT("Foo"), TEXT("SM_Foo")));
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Distinct/Bar"), TEXT("Bar"), TEXT("SM_Bar")));
		const FBertaAssetNamingBatchPreflightResult Result = BertaAssetNamingBatch::Preflight(Candidates, [](const FBertaAssetNamingBatchCandidate&)
		{
			return false;
		});
		TestTrue(TEXT("Distinct targets are accepted"), Result.IsSafe());
	}

	{
		TArray<FBertaAssetNamingBatchCandidate> Candidates;
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/FolderA/Rock"), TEXT("Rock"), TEXT("SM_Rock")));
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/FolderB/Rock"), TEXT("Rock"), TEXT("SM_Rock")));
		const FBertaAssetNamingBatchPreflightResult Result = BertaAssetNamingBatch::Preflight(Candidates, [](const FBertaAssetNamingBatchCandidate&)
		{
			return false;
		});
		TestTrue(TEXT("Matching names in different folders do not collide"), Result.IsSafe());
	}

	{
		TArray<FBertaAssetNamingBatchCandidate> Candidates;
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Duplicate/One"), TEXT("One"), TEXT("SM_Target")));
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Duplicate/Two"), TEXT("Two"), TEXT("SM_Target")));
		const FBertaAssetNamingBatchPreflightResult Result = BertaAssetNamingBatch::Preflight(Candidates, [](const FBertaAssetNamingBatchCandidate&)
		{
			return false;
		});
		TestFalse(TEXT("Duplicate complete target paths are rejected"), Result.IsSafe());
		TestEqual(TEXT("Every duplicate candidate is reported"), Result.Conflicts.Num(), 2);
	}

	{
		TArray<FBertaAssetNamingBatchCandidate> Candidates;
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Invalid/Source"), TEXT("Source"), TEXT("Invalid/Name")));
		const FBertaAssetNamingBatchPreflightResult Result = BertaAssetNamingBatch::Preflight(Candidates, [](const FBertaAssetNamingBatchCandidate&)
		{
			return false;
		});
		TestFalse(TEXT("Invalid target path is rejected"), Result.IsSafe());
		TestEqual(TEXT("Invalid target conflict count"), Result.Conflicts.Num(), 1);
		TestEqual(TEXT("Invalid target conflict type"), Result.Conflicts[0].Type, EBertaAssetNamingBatchConflictType::InvalidTarget);
	}

	{
		TArray<FBertaAssetNamingBatchCandidate> Candidates;
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Occupied/Source"), TEXT("Source"), TEXT("SM_Taken")));
		const FBertaAssetNamingBatchPreflightResult Result = BertaAssetNamingBatch::Preflight(Candidates, [](const FBertaAssetNamingBatchCandidate& Candidate)
		{
			return Candidate.TargetObjectPath == TEXT("/Game/AssetNamingBatch/Occupied/SM_Taken.SM_Taken");
		});
		TestFalse(TEXT("Occupied target is rejected"), Result.IsSafe());
		TestEqual(TEXT("Occupied target conflict type"), Result.Conflicts[0].Type, EBertaAssetNamingBatchConflictType::OccupiedTarget);
	}

	{
		TArray<FBertaAssetNamingBatchCandidate> Candidates;
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Chain/A"), TEXT("A"), TEXT("B")));
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Chain/B"), TEXT("B"), TEXT("C")));
		const FBertaAssetNamingBatchPreflightResult Result = BertaAssetNamingBatch::Preflight(Candidates, [&Candidates](const FBertaAssetNamingBatchCandidate& Candidate)
		{
			return Candidates.ContainsByPredicate([&Candidate](const FBertaAssetNamingBatchCandidate& ExistingCandidate)
			{
				return ExistingCandidate.SourceObjectPath == Candidate.TargetObjectPath;
			});
		});
		TestFalse(TEXT("Chained rename into a candidate source is rejected"), Result.IsSafe());
		TestEqual(TEXT("Chained rename conflict type"), Result.Conflicts[0].Type, EBertaAssetNamingBatchConflictType::OccupiedTarget);
	}

	return true;
}

#endif
