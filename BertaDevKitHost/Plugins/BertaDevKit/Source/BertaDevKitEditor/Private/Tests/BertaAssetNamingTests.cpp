#include "AssetActions/BertaAssetNamingUtils.h"
#include "AssetActions/BertaAssetNamingBatch.h"
#include "AssetActions/BertaAssetNamingValidator.h"
#include "ContentBrowser/BertaContentBrowserMenu.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimBlueprint.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/BlueprintSupport.h"
#include "Editor.h"
#include "EditorValidatorSubsystem.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/AutomationTest.h"
#include "Misc/DataValidation.h"
#include "Misc/PackageName.h"
#include "ToolMenus.h"

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

	FAssetData MakeAssetDataAtPackage(const TCHAR* PackageName, const TCHAR* AssetName)
	{
		const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);
		return FAssetData(FName(PackageName), FName(*PackagePath), FName(AssetName), UStaticMesh::StaticClass()->GetClassPathName());
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

	FAssetData MakeTransientAssetData(const TCHAR* PackageName, const TCHAR* AssetName, UClass* AssetClass, UObject*& OutAsset)
	{
		static int32 NextTestPackageId = 0;
		const FString UniquePackageName = FString::Printf(TEXT("%s_%d"), PackageName, ++NextTestPackageId);
		UPackage* Package = CreatePackage(*UniquePackageName);
		OutAsset = NewObject<UObject>(Package, AssetClass, AssetName, RF_Transient);
		return FAssetData(OutAsset);
	}

	EDataValidationResult ValidateAssetNaming(const FAssetData& AssetData, UObject* Asset, FDataValidationContext& OutContext)
	{
		UBertaAssetNamingValidator* Validator = NewObject<UBertaAssetNamingValidator>(GetTransientPackage());
		return Validator->ValidateLoadedAsset(AssetData, Asset, OutContext);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetNamingCoreTest, "BertaDevKit.AssetNaming.Core", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetNamingCoreTest::RunTest(const FString& Parameters)
{
	TestPlan(*this, MakeAssetData(TEXT("Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("SM_Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::AlreadyCorrect, TEXT("SM_"), TEXT("SM_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("M_Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("T_Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("BP_Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("XYZ_Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_XYZ_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("My_Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_My_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("M_T_Rock"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_T_Rock"));
	TestPlan(*this, MakeAssetData(TEXT("M_"), UStaticMesh::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("SM_"), TEXT("SM_M_"));
	TestPlan(*this, MakeAssetData(TEXT("Actor"), UBlueprint::StaticClass(), TEXT("Class'/Script/Engine.Actor'")), EBertaAssetNamingStatus::NeedsRename, TEXT("BP_"), TEXT("BP_Actor"));
	TestPlan(*this, MakeAssetData(TEXT("CH_Foo"), UBlueprint::StaticClass(), TEXT("Class'/Script/Engine.Actor'")), EBertaAssetNamingStatus::NeedsRename, TEXT("BP_"), TEXT("BP_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("Character"), UBlueprint::StaticClass(), TEXT("Class'/Script/Engine.Character'")), EBertaAssetNamingStatus::NeedsRename, TEXT("CH_"), TEXT("CH_Character"));
	TestPlan(*this, MakeAssetData(TEXT("BP_Hero"), UBlueprint::StaticClass(), TEXT("Class'/Script/Engine.Character'")), EBertaAssetNamingStatus::NeedsRename, TEXT("CH_"), TEXT("CH_Hero"));
	TestPlan(*this, MakeAssetData(TEXT("ChildCharacter"), UBlueprint::StaticClass(), TEXT("Class'/Script/Engine.Character'")), EBertaAssetNamingStatus::NeedsRename, TEXT("CH_"), TEXT("CH_ChildCharacter"));
	TestPlan(*this, MakeAssetData(TEXT("Widget"), UBlueprint::StaticClass(), TEXT("Class'/Script/UMG.UserWidget'")), EBertaAssetNamingStatus::NeedsRename, TEXT("WBP_"), TEXT("WBP_Widget"));
	TestPlan(*this, MakeAssetData(TEXT("Anim"), UAnimBlueprint::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("ABP_"), TEXT("ABP_Anim"));
	TestPlan(*this, MakeAssetData(TEXT("Ability"), UBlueprint::StaticClass(), TEXT("Class'/Script/GameplayAbilities.GameplayAbility'")), EBertaAssetNamingStatus::NeedsRename, TEXT("GA_"), TEXT("GA_Ability"));
	TestPlan(*this, MakeAssetData(TEXT("GE_Fireball"), UBlueprint::StaticClass(), TEXT("Class'/Script/GameplayAbilities.GameplayAbility'")), EBertaAssetNamingStatus::NeedsRename, TEXT("GA_"), TEXT("GA_Fireball"));
	TestPlan(*this, MakeAssetData(TEXT("AbilityBlueprint"), FTopLevelAssetPath(TEXT("/Script/GameplayAbilities"), TEXT("GameplayAbilityBlueprint"))), EBertaAssetNamingStatus::NeedsRename, TEXT("GA_"), TEXT("GA_AbilityBlueprint"));
	TestPlan(*this, MakeAssetData(TEXT("M_Foo_Inst"), UMaterialInstanceConstant::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("MI_"), TEXT("MI_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("T_Foo_Inst"), UMaterialInstanceConstant::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("MI_"), TEXT("MI_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("MI_Foo"), UMaterialInstanceConstant::StaticClass()), EBertaAssetNamingStatus::AlreadyCorrect, TEXT("MI_"), TEXT("MI_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("Foo_Montage"), UAnimMontage::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("AM_"), TEXT("AM_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("AS_Foo_Montage"), UAnimMontage::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("AM_"), TEXT("AM_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("Unknown"), UObject::StaticClass()), EBertaAssetNamingStatus::UnknownClass, TEXT(""), TEXT(""));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetNamingContentBrowserScopeTest, "BertaDevKit.AssetNaming.ContentBrowserScope", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetNamingContentBrowserScopeTest::RunTest(const FString& Parameters)
{
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenu* AssetContextMenu = UToolMenus::Get()->FindMenu(TEXT("ContentBrowser.AssetContextMenu"));
		TestNotNull(TEXT("Asset context menu is registered"), AssetContextMenu);
		if (AssetContextMenu)
		{
			TestNotNull(TEXT("Asset context menu contains BertaDevKit section"), AssetContextMenu->FindSection(TEXT("BertaDevKitAssetNaming")));
		}

		UToolMenu* FolderContextMenu = UToolMenus::Get()->FindMenu(TEXT("ContentBrowser.FolderContextMenu"));
		TestNotNull(TEXT("Folder context menu is registered"), FolderContextMenu);
		if (FolderContextMenu)
		{
			TestNotNull(TEXT("Folder context menu contains BertaDevKit section"), FolderContextMenu->FindSection(TEXT("BertaDevKitAssetNaming")));
		}
	}

	const FAssetData RootAsset = MakeAssetDataAtPackage(TEXT("/Game/MenuScope/Root/RootAsset"), TEXT("RootAsset"));
	const FAssetData ChildAsset = MakeAssetDataAtPackage(TEXT("/Game/MenuScope/Root/Child/ChildAsset"), TEXT("ChildAsset"));
	const FAssetData UnrelatedAsset = MakeAssetDataAtPackage(TEXT("/Game/MenuScope/Other/OtherAsset"), TEXT("OtherAsset"));
	const FAssetData EngineAsset = MakeAssetDataAtPackage(TEXT("/Engine/MenuScope/Root/EngineAsset"), TEXT("EngineAsset"));

	{
		TArray<FAssetData> ScopedAssets;
		FBertaContentBrowserMenu::FilterProjectAssets({ RootAsset, EngineAsset }, ScopedAssets);
		TestEqual(TEXT("Explicit asset scope excludes non-/Game assets"), ScopedAssets.Num(), 1);
		if (ScopedAssets.Num() == 1)
		{
			TestEqual(TEXT("Explicit asset scope does not expand selection"), ScopedAssets[0].GetObjectPathString(), RootAsset.GetObjectPathString());
		}
	}

	{
		TArray<FAssetData> ScopedAssets;
		FBertaContentBrowserMenu::FilterAssetsInProjectFolders({ TEXT("/Game/MenuScope/Root"), TEXT("/Game/MenuScope/Root/Child"), TEXT("/Engine/MenuScope/Root") }, { RootAsset, ChildAsset, ChildAsset, UnrelatedAsset, EngineAsset }, ScopedAssets);
		TestEqual(TEXT("Recursive folder scope includes root and child assets once"), ScopedAssets.Num(), 2);
		TestTrue(TEXT("Recursive folder scope includes root asset"), ScopedAssets.ContainsByPredicate([&RootAsset](const FAssetData& Asset) { return Asset.GetObjectPathString() == RootAsset.GetObjectPathString(); }));
		TestTrue(TEXT("Recursive folder scope includes child asset"), ScopedAssets.ContainsByPredicate([&ChildAsset](const FAssetData& Asset) { return Asset.GetObjectPathString() == ChildAsset.GetObjectPathString(); }));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetNamingDataValidationTest, "BertaDevKit.AssetNaming.DataValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetNamingDataValidationTest::RunTest(const FString& Parameters)
{
	if (UEditorValidatorSubsystem* ValidatorSubsystem = GEditor->GetEditorSubsystem<UEditorValidatorSubsystem>())
	{
		bool bFoundAssetNamingValidator = false;
		ValidatorSubsystem->ForEachEnabledValidator([&bFoundAssetNamingValidator](UEditorValidatorBase* Validator)
		{
			bFoundAssetNamingValidator |= Validator->IsA<UBertaAssetNamingValidator>();
			return !bFoundAssetNamingValidator;
		});
		TestTrue(TEXT("Asset Naming validator is auto-discovered"), bFoundAssetNamingValidator);
	}
	else
	{
		AddError(TEXT("Editor Validator Subsystem is unavailable."));
	}

	{
		UObject* Asset = nullptr;
		const FAssetData AssetData = MakeTransientAssetData(TEXT("/Game/BertaDevKitDataValidationTests/SM_Rock"), TEXT("SM_Rock"), UStaticMesh::StaticClass(), Asset);
		FDataValidationContext Context(false, EDataValidationUsecase::Manual, {});
		TestEqual(TEXT("Correct /Game Static Mesh is valid"), ValidateAssetNaming(AssetData, Asset, Context), EDataValidationResult::Valid);
	}

	{
		UObject* Asset = nullptr;
		const FAssetData AssetData = MakeTransientAssetData(TEXT("/Game/BertaDevKitDataValidationTests/Rock"), TEXT("Rock"), UStaticMesh::StaticClass(), Asset);
		FDataValidationContext Context(false, EDataValidationUsecase::Manual, {});
		TestEqual(TEXT("Unprefixed /Game Static Mesh is invalid"), ValidateAssetNaming(AssetData, Asset, Context), EDataValidationResult::Invalid);
		TestTrue(TEXT("Unprefixed Static Mesh error names target"), Context.GetIssues().ContainsByPredicate([](const FDataValidationContext::FIssue& Issue)
		{
			return Issue.Message.ToString().Contains(TEXT("SM_Rock"));
		}));
	}

	{
		UObject* Asset = nullptr;
		const FAssetData AssetData = MakeTransientAssetData(TEXT("/Game/BertaDevKitDataValidationTests/M_Rock"), TEXT("M_Rock"), UStaticMesh::StaticClass(), Asset);
		FDataValidationContext Context(false, EDataValidationUsecase::Manual, {});
		TestEqual(TEXT("Known wrong prefix is invalid"), ValidateAssetNaming(AssetData, Asset, Context), EDataValidationResult::Invalid);
		TestTrue(TEXT("Known wrong prefix error uses canonical planner target"), Context.GetIssues().ContainsByPredicate([](const FDataValidationContext::FIssue& Issue)
		{
			return Issue.Message.ToString().Contains(TEXT("SM_Rock"));
		}));
	}

	{
		UObject* Asset = nullptr;
		const FAssetData AssetData = MakeTransientAssetData(TEXT("/Engine/BertaDevKitDataValidationTests/Rock"), TEXT("Rock"), UStaticMesh::StaticClass(), Asset);
		FDataValidationContext Context(false, EDataValidationUsecase::Manual, {});
		TestEqual(TEXT("Known asset outside /Game is not validated"), ValidateAssetNaming(AssetData, Asset, Context), EDataValidationResult::NotValidated);
	}

	{
		UObject* Asset = nullptr;
		const FAssetData ConcreteAssetData = MakeTransientAssetData(TEXT("/Game/BertaDevKitDataValidationTests/Unknown"), TEXT("Unknown"), UStaticMesh::StaticClass(), Asset);
		const FAssetData AssetData(ConcreteAssetData.PackageName, ConcreteAssetData.PackagePath, ConcreteAssetData.AssetName, UObject::StaticClass()->GetClassPathName());
		FDataValidationContext Context(false, EDataValidationUsecase::Manual, {});
		TestEqual(TEXT("Unknown /Game asset class is not validated"), ValidateAssetNaming(AssetData, Asset, Context), EDataValidationResult::NotValidated);
	}

	{
		UObject* Asset = nullptr;
		const FAssetData AssetData = MakeTransientAssetData(TEXT("/Game/BertaDevKitDataValidationTests/SaveRock"), TEXT("Rock"), UStaticMesh::StaticClass(), Asset);
		FDataValidationContext SaveContext(false, EDataValidationUsecase::Save, {});
		TestEqual(TEXT("Naming does not validate on save"), ValidateAssetNaming(AssetData, Asset, SaveContext), EDataValidationResult::NotValidated);

		FDataValidationContext ManualContext(false, EDataValidationUsecase::Manual, {});
		TestEqual(TEXT("Naming validates manually"), ValidateAssetNaming(AssetData, Asset, ManualContext), EDataValidationResult::Invalid);
	}

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
		if (Result.Conflicts.Num() == 1)
		{
			TestEqual(TEXT("Invalid target conflict type"), Result.Conflicts[0].Type, EBertaAssetNamingBatchConflictType::InvalidTarget);
		}
	}

	{
		TArray<FBertaAssetNamingBatchCandidate> Candidates;
		Candidates.Add(MakeBatchCandidate(*this, TEXT("/Game/AssetNamingBatch/Occupied/Source"), TEXT("Source"), TEXT("SM_Taken")));
		const FBertaAssetNamingBatchPreflightResult Result = BertaAssetNamingBatch::Preflight(Candidates, [](const FBertaAssetNamingBatchCandidate& Candidate)
		{
			return Candidate.TargetObjectPath == TEXT("/Game/AssetNamingBatch/Occupied/SM_Taken.SM_Taken");
		});
		TestFalse(TEXT("Occupied target is rejected"), Result.IsSafe());
		TestEqual(TEXT("Occupied target conflict count"), Result.Conflicts.Num(), 1);
		if (Result.Conflicts.Num() == 1)
		{
			TestEqual(TEXT("Occupied target conflict type"), Result.Conflicts[0].Type, EBertaAssetNamingBatchConflictType::OccupiedTarget);
		}
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
		TestEqual(TEXT("Chained rename conflict count"), Result.Conflicts.Num(), 1);
		if (Result.Conflicts.Num() == 1)
		{
			TestEqual(TEXT("Chained rename conflict type"), Result.Conflicts[0].Type, EBertaAssetNamingBatchConflictType::OccupiedTarget);
		}
	}

	return true;
}

#endif
