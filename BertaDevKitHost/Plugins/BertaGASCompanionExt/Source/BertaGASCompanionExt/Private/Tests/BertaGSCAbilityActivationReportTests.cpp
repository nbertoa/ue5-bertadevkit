#include "Abilities/BertaGSCAbilityActivationReport.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCAbilityActivationReportFormattingTest,
	"BertaDevKit.GASCompanionExt.AbilityActivationReport.Formatting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCAbilityActivationReportFormattingTest::RunTest(const FString& Parameters)
{
	FBertaGSCAbilityActivationReport Report;
	Report.ActorPath = TEXT("/Game/TestActor");
	Report.AbilityClassPath = TEXT("/Game/GA_Test.GA_Test_C");
	Report.bAbilitySystemComponentFound = true;
	Report.bExactAbilityGranted = true;
	Report.GrantedLevel = 3;
	Report.bCostCheckAvailable = true;
	Report.Cost.bCanPayCost = true;
	Report.bCooldownCheckAvailable = true;
	Report.bActivationCheckAvailable = true;
	Report.Activation.bCanActivate = true;

	const FString Summary = UBertaGSCAbilityActivationLibrary::FormatAbilityActivationReport(Report);
	TestTrue(TEXT("Includes actor identity"), Summary.Contains(TEXT("Actor=/Game/TestActor")));
	TestTrue(TEXT("Includes exact ability identity"), Summary.Contains(TEXT("Ability=/Game/GA_Test.GA_Test_C")));
	TestTrue(TEXT("Includes deterministic level"), Summary.Contains(TEXT("Level=3")));
	TestTrue(TEXT("Includes activation result"), Summary.Contains(TEXT("CanActivate=Yes")));
	return true;
}

#endif
