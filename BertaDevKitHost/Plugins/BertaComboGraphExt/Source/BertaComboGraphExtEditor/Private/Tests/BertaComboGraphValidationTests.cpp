#if WITH_DEV_AUTOMATION_TESTS

#include "Validation/BertaComboGraphValidation.h"
#include "Validation/BertaComboGraphValidationRules.h"

#include "Graph/ComboGraph.h"
#include "Graph/ComboGraphEdge.h"
#include "Graph/ComboGraphNodeEntry.h"
#include "Graph/ComboGraphNodeMontage.h"
#include "Misc/AutomationTest.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComboGraphValidationRangeTest,
	"Berta.ComboGraph.Validation.ComboWindowRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComboGraphValidationRangeTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Nominal normalized range"), UBertaComboGraphValidationLibrary::IsValidNormalizedRange(0.25f, 0.75f));
	TestFalse(TEXT("Equal endpoints"), UBertaComboGraphValidationLibrary::IsValidNormalizedRange(0.5f, 0.5f));
	TestFalse(TEXT("Outside normalized domain"), UBertaComboGraphValidationLibrary::IsValidNormalizedRange(-0.1f, 0.8f));
	TestFalse(TEXT("Non-finite range"), UBertaComboGraphValidationLibrary::IsValidNormalizedRange(0.2f, std::numeric_limits<float>::infinity()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComboGraphValidationOrderingTest,
	"Berta.ComboGraph.Validation.DeterministicOrdering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComboGraphValidationOrderingTest::RunTest(const FString& Parameters)
{
	FBertaComboGraphValidationReport Report;
	FBertaComboGraphValidationFinding Warning;
	Warning.Severity = EBertaComboGraphValidationSeverity::Warning;
	Warning.Code = TEXT("CG.Zeta");
	Report.Findings.Add(Warning);
	FBertaComboGraphValidationFinding Error;
	Error.Severity = EBertaComboGraphValidationSeverity::Error;
	Error.Code = TEXT("CG.Alpha");
	Report.Findings.Add(Error);
	FBertaComboGraphValidationFinding Error2 = Error;
	Error2.Code = TEXT("CG.Beta");
	Report.Findings.Add(Error2);
	UBertaComboGraphValidationLibrary::SortAndCount(Report);
	TestEqual(TEXT("Counts errors"), Report.ErrorCount, 2);
	TestEqual(TEXT("Counts warnings"), Report.WarningCount, 1);
	TestEqual(TEXT("Errors sort first"), Report.Findings[0].Code, FString(TEXT("CG.Alpha")));
	TestEqual(TEXT("Stable code ordering within severity"), Report.Findings[1].Code, FString(TEXT("CG.Beta")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComboGraphTopologyRulesTest,
	"Berta.ComboGraph.Validation.TopologyRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComboGraphTopologyRulesTest::RunTest(const FString& Parameters)
{
	TMap<FString, TArray<FString>> Acyclic;
	Acyclic.Add(TEXT("Entry"), {TEXT("A")});
	Acyclic.Add(TEXT("A"), {TEXT("B")});
	Acyclic.Add(TEXT("B"));
	TestFalse(TEXT("Acyclic topology"), BertaComboGraphValidationRules::HasCycle(Acyclic));
	const TSet<FString> Reachable = BertaComboGraphValidationRules::ReachableFrom(Acyclic, TEXT("Entry"));
	TestTrue(TEXT("Reachability includes descendants"), Reachable.Contains(TEXT("B")));

	TMap<FString, TArray<FString>> Cyclic = Acyclic;
	Cyclic[TEXT("B")].Add(TEXT("A"));
	TestTrue(TEXT("Back edge is a cycle"), BertaComboGraphValidationRules::HasCycle(Cyclic));
	TestTrue(TEXT("Duplicate signature detected"), BertaComboGraphValidationRules::HasDuplicateExactSignatures({TEXT("IA_Attack|Triggered"), TEXT("IA_Attack|Triggered")}));
	TestFalse(TEXT("Distinct trigger signature accepted"), BertaComboGraphValidationRules::HasDuplicateExactSignatures({TEXT("IA_Attack|Started"), TEXT("IA_Attack|Triggered")}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComboGraphMissingInputFindingTest,
	"Berta.ComboGraph.Validation.MissingTransitionInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComboGraphMissingInputFindingTest::RunTest(const FString& Parameters)
{
	UComboGraph* Graph = NewObject<UComboGraph>();
	UComboGraphNodeEntry* Entry = NewObject<UComboGraphNodeEntry>(Graph);
	UComboGraphNodeMontage* First = NewObject<UComboGraphNodeMontage>(Graph);
	UComboGraphNodeMontage* Second = NewObject<UComboGraphNodeMontage>(Graph);
	UComboGraphEdge* EntryEdge = NewObject<UComboGraphEdge>(Graph);
	UComboGraphEdge* MissingInputEdge = NewObject<UComboGraphEdge>(Graph);

	Graph->EntryNode = Entry;
	Graph->FirstNode = First;
	Graph->AllNodes = {Entry, First, Second};
	Entry->ChildrenNodes.Add(First);
	First->ParentNodes.Add(Entry);
	EntryEdge->StartNode = Entry;
	EntryEdge->EndNode = First;
	Entry->Edges.Add(First, EntryEdge);
	First->ChildrenNodes.Add(Second);
	Second->ParentNodes.Add(First);
	MissingInputEdge->StartNode = First;
	MissingInputEdge->EndNode = Second;
	First->Edges.Add(Second, MissingInputEdge);

	FBertaComboGraphValidationReport Report;
	TestTrue(TEXT("Transient graph can be inspected"), UBertaComboGraphValidationLibrary::BuildValidationReport(Graph, Report));
	TestTrue(TEXT("Missing transition input has stable error code"), Report.Findings.ContainsByPredicate([](const FBertaComboGraphValidationFinding& Finding)
	{
		return Finding.Code == TEXT("CG.MissingTransitionInput") && Finding.Severity == EBertaComboGraphValidationSeverity::Error;
	}));
	return true;
}

#endif
