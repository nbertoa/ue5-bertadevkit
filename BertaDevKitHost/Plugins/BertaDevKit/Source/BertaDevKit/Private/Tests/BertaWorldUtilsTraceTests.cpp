#include "World/BertaWorldUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace
{
	void PopulateHit(FHitResult& OutHit)
	{
		OutHit.bBlockingHit = true;
		OutHit.Distance = 42.0f;
	}

	void TestClearedHit(FAutomationTestBase& Test, const FHitResult& Hit)
	{
		Test.TestTrue(TEXT("OutHit is not blocking"), !Hit.bBlockingHit);
		Test.TestTrue(TEXT("OutHit distance is reset"), Hit.Distance == 0.0f);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaWorldLineTraceInvalidContextTest,
	"BertaDevKit.World.Traces.LineTrace.InvalidContextClearsHit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaWorldLineTraceInvalidContextTest::RunTest(const FString& Parameters)
{
	FHitResult Hit;
	PopulateHit(Hit);
	AddExpectedError(TEXT("WorldContextObject is null or pending kill"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("LineTrace fails without a world context"), UBertaWorldUtils::LineTrace(nullptr, FVector::ZeroVector, FVector::ForwardVector, ECC_Visibility, {}, Hit));
	TestClearedHit(*this, Hit);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaWorldSphereTraceInvalidContextTest,
	"BertaDevKit.World.Traces.SphereTrace.InvalidContextClearsHit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaWorldSphereTraceInvalidContextTest::RunTest(const FString& Parameters)
{
	FHitResult Hit;
	PopulateHit(Hit);
	AddExpectedError(TEXT("WorldContextObject is null or pending kill"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("SphereTrace fails without a world context"), UBertaWorldUtils::SphereTrace(nullptr, FVector::ZeroVector, FVector::ForwardVector, 1.0f, ECC_Visibility, {}, Hit));
	TestClearedHit(*this, Hit);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaWorldSphereTraceNegativeRadiusTest,
	"BertaDevKit.World.Traces.SphereTrace.NegativeRadiusClearsHit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaWorldSphereTraceNegativeRadiusTest::RunTest(const FString& Parameters)
{
	FHitResult Hit;
	PopulateHit(Hit);
	AddExpectedError(TEXT("Radius must be non-negative"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("SphereTrace rejects a negative radius"), UBertaWorldUtils::SphereTrace(nullptr, FVector::ZeroVector, FVector::ForwardVector, -1.0f, ECC_Visibility, {}, Hit));
	TestClearedHit(*this, Hit);
	return true;
}

#endif
