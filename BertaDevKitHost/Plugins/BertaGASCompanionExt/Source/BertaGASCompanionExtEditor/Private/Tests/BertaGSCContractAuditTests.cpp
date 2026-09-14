#include "Validation/BertaGSCContractAudit.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Validation/BertaGSCContractAuditInternal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCContractAuditPureLogicTest,
	"BertaDevKit.GASCompanionExt.ContractAudit.PureLogic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCContractAuditPureLogicTest::RunTest(const FString& Parameters)
{
	const BertaGSCContractAudit::FAbilityContractRecord Base{
		TEXT("/Game/GA_Test.GA_Test_C"), TEXT("/Game/IA_Test.IA_Test"), 1, 0, TEXT("GrantedAbilities[0]")};
	const BertaGSCContractAudit::FAbilityContractRecord Duplicate{
		Base.ClassPath, Base.InputPath, Base.Level, Base.Trigger, TEXT("GrantedAbilities[1]")};
	const BertaGSCContractAudit::FAbilityContractRecord ConflictingLevel{
		Base.ClassPath, Base.InputPath, 2, Base.Trigger, TEXT("GrantedAbilities[2]")};
	const BertaGSCContractAudit::FAbilityContractRecord SharedInput{
		TEXT("/Game/GA_Other.GA_Other_C"), Base.InputPath, 1, 0, TEXT("GrantedAbilities[3]")};
	TestTrue(TEXT("Exact duplicates ignore only property location"), BertaGSCContractAudit::IsExactDuplicate(Base, Duplicate));
	TestTrue(TEXT("Same ability with another level conflicts"), BertaGSCContractAudit::HasConflictingSameAbilityConfiguration(Base, ConflictingLevel));
	TestTrue(TEXT("Same input across abilities is detected"), BertaGSCContractAudit::SharesInputAcrossDifferentAbilities(Base, SharedInput));
	TestFalse(TEXT("Inferred similarity is not exact duplication"), BertaGSCContractAudit::IsExactDuplicate(Base, ConflictingLevel));

	FBertaGSCContractAuditReport Report;
	BertaGSCContractAudit::AddFinding(
		Report,
		EBertaGSCContractFindingSeverity::Warning,
		TEXT("/Game/Z"),
		TEXT("GrantedAbilities[2]"),
		TEXT("ConflictingAbilityMapping"),
		TEXT("Conflicting level"),
		TEXT("Level 1 vs 2"));
	BertaGSCContractAudit::AddFinding(
		Report,
		EBertaGSCContractFindingSeverity::Error,
		TEXT("/Game/A"),
		TEXT("GrantedAbilities[1]"),
		TEXT("DuplicateAbilityMapping"),
		TEXT("Exact duplicate"));
	BertaGSCContractAudit::Finalize(Report);

	TestEqual(TEXT("Errors are counted"), Report.ErrorCount, 1);
	TestEqual(TEXT("Warnings are counted"), Report.WarningCount, 1);
	TestEqual(TEXT("Severity participates in deterministic ordering"), Report.Findings[0].Severity, EBertaGSCContractFindingSeverity::Error);
	TestTrue(TEXT("Stable key contains cross-reference code"), Report.Findings[1].StableKey.Contains(TEXT("ConflictingAbilityMapping")));
	TestTrue(TEXT("Formatter retains evidence"), Report.Summary.Contains(TEXT("Level 1 vs 2")));
	return true;
}

#endif
