#include "Abilities/BertaGSCAbilityReadiness.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Abilities/BertaGSCAbilityReadinessInternal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCAbilityReadinessPureLogicTest,
	"BertaDevKit.GASCompanionExt.AbilityReadiness.PureLogic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCAbilityReadinessPureLogicTest::RunTest(const FString& Parameters)
{
	FBertaGSCAbilityReadinessEntry Ready;
	Ready.Inspection.AbilityClassPath = TEXT("/Game/GA_Z.GA_Z_C");
	Ready.Inspection.bCostCheckAvailable = true;
	Ready.Inspection.Cost.bCanPayCost = true;
	Ready.Inspection.bCooldownCheckAvailable = true;
	Ready.Inspection.bActivationCheckAvailable = true;
	Ready.Inspection.Activation.bCanActivate = true;
	BertaGSCAbilityReadiness::Classify(Ready);
	TestTrue(TEXT("Fully available inactive ability is ready"), Ready.bReady);
	TestFalse(TEXT("Ready ability is not a problem"), Ready.bHasProblem);

	FBertaGSCAbilityReadinessEntry Blocked = Ready;
	Blocked.Inspection.AbilityClassPath = TEXT("/Game/GA_A.GA_A_C");
	Blocked.Inspection.Cost.bCanPayCost = false;
	Blocked.Inspection.Cooldown.bIsOnCooldown = true;
	Blocked.Inspection.Activation.bCanActivate = false;
	BertaGSCAbilityReadiness::Classify(Blocked);
	TestTrue(TEXT("Simultaneous cost blocker is retained"), Blocked.States.Contains(EBertaGSCAbilityReadinessState::BlockedByCost));
	TestTrue(TEXT("Simultaneous cooldown blocker is retained"), Blocked.States.Contains(EBertaGSCAbilityReadinessState::BlockedByCooldown));
	TestTrue(TEXT("Simultaneous activation blocker is retained"), Blocked.States.Contains(EBertaGSCAbilityReadinessState::BlockedByActivation));

	TArray<FBertaGSCAbilityReadinessEntry> Entries{Ready, Blocked};
	BertaGSCAbilityReadiness::SortEntries(Entries);
	TestEqual(TEXT("Entries sort by exact class path"), Entries[0].Inspection.AbilityClassPath, Blocked.Inspection.AbilityClassPath);

	FBertaGSCAbilityReadinessReport Report;
	Report.ActorPath = TEXT("/Game/TestActor");
	Report.Entries = Entries;
	Report.ProblemCount = 1;
	const FString Problems = UBertaGSCAbilityReadinessLibrary::FormatAbilityReadinessProblems(Report);
	TestTrue(TEXT("Problems formatter includes blocked ability"), Problems.Contains(Blocked.Inspection.AbilityClassPath));
	TestFalse(TEXT("Problems formatter excludes ready ability"), Problems.Contains(Ready.Inspection.AbilityClassPath));
	return true;
}

#endif
