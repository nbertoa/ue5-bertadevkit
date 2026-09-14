#include "Validation/BertaGSCAbilitySetValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCAbilitySetEffectLevelValidationTest,
	"BertaDevKit.GASCompanionExt.AbilitySetValidator.EffectLevel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCAbilitySetEffectLevelValidationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Zero remains a valid native Gameplay Effect level"), BertaGSCAbilitySetValidation::IsGameplayEffectLevelValid(0.0f));
	TestTrue(TEXT("Negative finite levels are not rejected without a GSC contract"), BertaGSCAbilitySetValidation::IsGameplayEffectLevelValid(-1.0f));
	TestFalse(TEXT("NaN is invalid"), BertaGSCAbilitySetValidation::IsGameplayEffectLevelValid(std::numeric_limits<float>::quiet_NaN()));
	TestFalse(TEXT("Infinity is invalid"), BertaGSCAbilitySetValidation::IsGameplayEffectLevelValid(std::numeric_limits<float>::infinity()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCAbilitySetInitializationDataValidationTest,
	"BertaDevKit.GASCompanionExt.AbilitySetValidator.InitializationData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCAbilitySetInitializationDataValidationTest::RunTest(const FString& Parameters)
{
	const TSoftObjectPtr<UDataTable> EmptyInitializationData;
	TestTrue(
		TEXT("Unconfigured InitializationData is valid"),
		BertaGSCAbilitySetValidation::IsInitializationDataResolvable(EmptyInitializationData));

	UDataTable* LoadedTable = NewObject<UDataTable>(GetTransientPackage());
	const TSoftObjectPtr<UDataTable> LoadedInitializationData(LoadedTable);
	TestTrue(
		TEXT("A configured loaded DataTable resolves"),
		BertaGSCAbilitySetValidation::IsInitializationDataResolvable(LoadedInitializationData));

	const TSoftObjectPtr<UDataTable> BrokenInitializationData(
		FSoftObjectPath(TEXT("/Game/BertaDevKitTests/MissingInitializationData.MissingInitializationData")));
	TestFalse(
		TEXT("A configured missing DataTable does not resolve"),
		BertaGSCAbilitySetValidation::IsInitializationDataResolvable(BrokenInitializationData));
	return true;
}

#endif
