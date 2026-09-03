#include "AssetActions/BertaAssetNamingUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimBlueprint.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/BlueprintSupport.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/AutomationTest.h"

namespace
{
	FAssetData MakeAssetData(FName Name, UClass* Class, const FString& NativeParentPath = FString())
	{
		FAssetDataTagMap Tags;
		if (!NativeParentPath.IsEmpty()) Tags.Add(FBlueprintTags::NativeParentClassPath, NativeParentPath);
		return FAssetData(FName(TEXT("/Game/AssetNamingTests")), FName(TEXT("/Game")), Name, Class->GetClassPathName(), MoveTemp(Tags));
	}
	void TestPlan(FAutomationTestBase& Test, const FAssetData& Asset, EBertaAssetNamingStatus Status, const TCHAR* Prefix, const TCHAR* Target)
	{
		const FBertaAssetNamingPlan Plan = UBertaAssetNamingUtils::BuildRenamePlan(Asset);
		Test.TestEqual(TEXT("Status"), Plan.Status, Status);
		Test.TestEqual(TEXT("Prefix"), Plan.ExpectedPrefix, FString(Prefix));
		Test.TestEqual(TEXT("Target"), Plan.TargetName, FString(Target));
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
	TestPlan(*this, MakeAssetData(TEXT("Anim"), UAnimBlueprint::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("ABP_"), TEXT("ABP_Anim"));
	TestPlan(*this, MakeAssetData(TEXT("Ability"), UBlueprint::StaticClass(), TEXT("Class'/Script/GameplayAbilities.GameplayAbility'")), EBertaAssetNamingStatus::NeedsRename, TEXT("GA_"), TEXT("GA_Ability"));
	TestPlan(*this, MakeAssetData(TEXT("M_Foo_Inst"), UMaterialInstanceConstant::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("MI_"), TEXT("MI_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("Foo_Montage"), UAnimMontage::StaticClass()), EBertaAssetNamingStatus::NeedsRename, TEXT("AM_"), TEXT("AM_Foo"));
	TestPlan(*this, MakeAssetData(TEXT("Unknown"), UObject::StaticClass()), EBertaAssetNamingStatus::UnknownClass, TEXT(""), TEXT(""));
	return true;
}

#endif
