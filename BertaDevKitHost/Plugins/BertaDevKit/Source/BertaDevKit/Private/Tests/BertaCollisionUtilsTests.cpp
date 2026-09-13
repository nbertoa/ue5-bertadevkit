#include "Collision/BertaCollisionUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaCollisionUtilsNullInputsTest,
	"BertaDevKit.Collision.Debug.NullInputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaCollisionUtilsNullInputsTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Two null components produce an unavailable pair"),
	          UBertaCollisionUtils::GetCollisionPairDebugSummary(nullptr, nullptr),
	          FString(TEXT("A: None\n\nB: None\n\nPair:\nUnavailable")));

	UBoxComponent* B = NewObject<UBoxComponent>();
	const FString NullASummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(nullptr, B);
	TestTrue(TEXT("A null input is identified"), NullASummary.Contains(TEXT("A: None")));
	TestTrue(TEXT("A valid B without an owner includes its identity"),
	         NullASummary.Contains(FString::Printf(TEXT("B: %s [Owner: None]"), *B->GetName())));
	TestTrue(TEXT("A missing component makes the pair unavailable"), NullASummary.Contains(TEXT("Pair:\nUnavailable")));
	TestTrue(TEXT("A missing component produces a factual note"), NullASummary.Contains(TEXT("A is None.")));

	AActor* Owner = NewObject<AActor>();
	UBoxComponent* A = NewObject<UBoxComponent>(Owner);
	const FString NullBSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, nullptr);
	TestTrue(TEXT("A valid component includes its owner"),
	         NullBSummary.Contains(FString::Printf(TEXT("A: %s [%s]"), *A->GetName(), *Owner->GetName())));
	TestTrue(TEXT("B null input is identified"), NullBSummary.Contains(TEXT("B: None")));
	TestTrue(TEXT("B missing component produces a factual note"), NullBSummary.Contains(TEXT("B is None.")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaCollisionUtilsPairResponsesTest,
	"BertaDevKit.Collision.Debug.PairResponses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaCollisionUtilsPairResponsesTest::RunTest(const FString& Parameters)
{
	UBoxComponent* A = NewObject<UBoxComponent>();
	USphereComponent* B = NewObject<USphereComponent>();
	A->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	B->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	A->SetCollisionObjectType(ECC_Pawn);
	B->SetCollisionObjectType(ECC_WorldDynamic);
	A->SetGenerateOverlapEvents(true);
	B->SetGenerateOverlapEvents(true);

	A->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	A->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	B->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	B->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	const FString CrossChannelSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, B);
	TestTrue(TEXT("A response is looked up against B's object type"),
	         CrossChannelSummary.Contains(TEXT("ResponseToB(WorldDynamic): Overlap")));
	TestTrue(TEXT("B response is looked up against A's object type"),
	         CrossChannelSummary.Contains(TEXT("ResponseToA(Pawn): Block")));
	TestTrue(TEXT("Overlap and Block resolve to Overlap"),
	         CrossChannelSummary.Contains(TEXT("ResolvedInteraction: Overlap")));

	B->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	const FString OverlapSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, B);
	TestTrue(TEXT("Two Overlap responses resolve to Overlap"),
	         OverlapSummary.Contains(TEXT("ResolvedInteraction: Overlap")));
	TestTrue(TEXT("A valid Overlap pair reports no obvious configuration blocker"),
	         OverlapSummary.Contains(TEXT("No obvious overlap-configuration blocker found.")));
	TestTrue(TEXT("A valid Overlap pair preserves the geometry and lifecycle caveat"),
	         OverlapSummary.Contains(TEXT("Geometry, movement, registration, lifecycle, and timing")));

	A->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	B->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	const FString BlockSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, B);
	TestTrue(TEXT("Two Block responses resolve to Block"),
	         BlockSummary.Contains(TEXT("ResolvedInteraction: Block")));
	TestTrue(TEXT("A Block pair scopes hit-event diagnostics out of V1"),
	         BlockSummary.Contains(TEXT("Hit-event notification settings are outside this V1 diagnostic.")));

	A->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	const FString AIgnoreSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, B);
	TestTrue(TEXT("A Ignore response resolves the pair to Ignore"),
	         AIgnoreSummary.Contains(TEXT("ResolvedInteraction: Ignore")));
	TestTrue(TEXT("The Ignore note identifies A and B's channel"),
	         AIgnoreSummary.Contains(TEXT("A ignores B's WorldDynamic channel.")));

	A->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	B->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	const FString BIgnoreSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, B);
	TestTrue(TEXT("B Ignore response resolves the pair to Ignore"),
	         BIgnoreSummary.Contains(TEXT("ResolvedInteraction: Ignore")));
	TestTrue(TEXT("The Ignore note identifies B and A's channel"),
	         BIgnoreSummary.Contains(TEXT("B ignores A's Pawn channel.")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaCollisionUtilsDiagnosticsTest,
	"BertaDevKit.Collision.Debug.Diagnostics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaCollisionUtilsDiagnosticsTest::RunTest(const FString& Parameters)
{
	UBoxComponent* A = NewObject<UBoxComponent>();
	USphereComponent* B = NewObject<USphereComponent>();
	A->SetCollisionObjectType(ECC_Pawn);
	B->SetCollisionObjectType(ECC_WorldDynamic);
	A->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	B->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	A->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	B->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	A->SetGenerateOverlapEvents(true);
	B->SetGenerateOverlapEvents(false);

	const FString DisabledOverlapEventsSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, B);
	TestTrue(TEXT("Disabled overlap events identify the component"),
	         DisabledOverlapEventsSummary.Contains(TEXT("B does not generate overlap events.")));
	TestTrue(TEXT("Disabled overlap events explain the bilateral requirement"),
	         DisabledOverlapEventsSummary.Contains(TEXT("Both components must enable GenerateOverlapEvents")));

	B->SetGenerateOverlapEvents(true);
	A->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	const FString QueryDisabledSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, B);
	TestTrue(TEXT("PhysicsOnly uses a stable collision-enabled label"),
	         QueryDisabledSummary.Contains(TEXT("CollisionEnabled: PhysicsOnly")));
	TestTrue(TEXT("PhysicsOnly produces a query participation note"),
	         QueryDisabledSummary.Contains(TEXT("A uses PhysicsOnly and does not participate in query collision.")));

	A->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TestTrue(TEXT("NoCollision uses a stable label"),
	         UBertaCollisionUtils::GetCollisionPairDebugSummary(A, nullptr).Contains(TEXT("CollisionEnabled: NoCollision")));
	A->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TestTrue(TEXT("QueryOnly uses a stable label"),
	         UBertaCollisionUtils::GetCollisionPairDebugSummary(A, nullptr).Contains(TEXT("CollisionEnabled: QueryOnly")));
	A->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	TestTrue(TEXT("PhysicsOnly uses a stable label"),
	         UBertaCollisionUtils::GetCollisionPairDebugSummary(A, nullptr).Contains(TEXT("CollisionEnabled: PhysicsOnly")));
	A->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TestTrue(TEXT("QueryAndPhysics uses a stable label"),
	         UBertaCollisionUtils::GetCollisionPairDebugSummary(A, nullptr).Contains(TEXT("CollisionEnabled: QueryAndPhysics")));
	A->SetCollisionEnabled(ECollisionEnabled::ProbeOnly);
	TestTrue(TEXT("ProbeOnly uses a stable label"),
	         UBertaCollisionUtils::GetCollisionPairDebugSummary(A, nullptr).Contains(TEXT("CollisionEnabled: ProbeOnly")));
	A->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
	const FString QueryAndProbeSummary = UBertaCollisionUtils::GetCollisionPairDebugSummary(A, B);
	TestTrue(TEXT("QueryAndProbe uses a stable label"),
	         QueryAndProbeSummary.Contains(TEXT("CollisionEnabled: QueryAndProbe")));
	TestTrue(TEXT("Built-in object channels use collision-profile names"),
	         QueryAndProbeSummary.Contains(TEXT("ObjectType: Pawn"))
	         && QueryAndProbeSummary.Contains(TEXT("ObjectType: WorldDynamic")));
	TestTrue(TEXT("Common collision responses use stable labels"),
	         QueryAndProbeSummary.Contains(TEXT("ResponseToB(WorldDynamic): Overlap"))
	         && QueryAndProbeSummary.Contains(TEXT("ResponseToA(Pawn): Overlap")));
	return true;
}

#endif
