#include "Validation/BertaGSCAbilitySetValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

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

#endif
