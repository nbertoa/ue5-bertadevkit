#include "Diagnostics/BertaGSCEffectiveLoadout.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Diagnostics/BertaGSCEffectiveLoadoutInternal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaGSCEffectiveLoadoutPureLogicTest,
	"BertaDevKit.GASCompanionExt.EffectiveLoadout.PureLogic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaGSCEffectiveLoadoutPureLogicTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Proven is explicit"), UBertaGSCEffectiveLoadoutLibrary::FormatProvenanceConfidence(EBertaGSCProvenanceConfidence::Proven), FString(TEXT("Proven")));
	TestEqual(TEXT("Inferred is never promoted"), UBertaGSCEffectiveLoadoutLibrary::FormatProvenanceConfidence(EBertaGSCProvenanceConfidence::Inferred), FString(TEXT("Inferred")));
	TestEqual(TEXT("Unknown remains explicit"), UBertaGSCEffectiveLoadoutLibrary::FormatProvenanceConfidence(EBertaGSCProvenanceConfidence::Unknown), FString(TEXT("Unknown")));

	FBertaGSCEffectiveLoadoutSnapshot Snapshot;
	FBertaGSCLoadoutAttributeSet Z;
	Z.AttributeSetClassPath = TEXT("/Game/Z.Z_C");
	FBertaGSCLoadoutAttributeSet A;
	A.AttributeSetClassPath = TEXT("/Game/A.A_C");
	Snapshot.AttributeSets = {Z, A};
	BertaGSCEffectiveLoadout::SortSnapshot(Snapshot);
	TestEqual(TEXT("Snapshot sorting is deterministic"), Snapshot.AttributeSets[0].AttributeSetClassPath, A.AttributeSetClassPath);

	FBertaGSCLoadoutAbility UnknownAbility;
	UnknownAbility.Readiness.Inspection.AbilityClassPath = TEXT("/Game/GA_Unknown.GA_Unknown_C");
	Snapshot.Abilities.Add(UnknownAbility);
	const FString Text = UBertaGSCEffectiveLoadoutLibrary::FormatEffectiveLoadoutSnapshot(Snapshot);
	TestTrue(TEXT("Missing evidence formats as Unknown"), Text.Contains(TEXT("Provenance: Unknown")));
	return true;
}

#endif
