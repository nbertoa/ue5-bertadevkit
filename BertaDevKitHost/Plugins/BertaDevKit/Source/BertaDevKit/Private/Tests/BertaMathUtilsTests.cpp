#include "Math/BertaMathUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaMathUtilsProjectileImpactTest,
	"BertaDevKit.Math.ProjectileImpactPoint.SelectsFirstCrossing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaMathUtilsProjectileImpactTest::RunTest(const FString& Parameters)
{
	FVector ImpactPoint;
	const bool bReachedPlane = UBertaMathUtils::ProjectileImpactPoint(FVector::ZeroVector,
	                                                                  FVector(1.0f, 0.0f, 1.0f),
	                                                                  100.0f,
	                                                                  10.0f,
	                                                                  -10.0f,
	                                                                  ImpactPoint);

	TestTrue(TEXT("The projectile reaches the plane"), bReachedPlane);
	TestTrue(TEXT("The returned point is the ascending, first crossing"), ImpactPoint.X < 100.0f);
	TestTrue(TEXT("The impact point lies on the requested plane"),
	         FMath::IsNearlyEqual(ImpactPoint.Z, 10.0f, 0.01f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaMathUtilsArcTest,
	"BertaDevKit.Math.IsAngleInArc.ClampsInvalidHalfArc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaMathUtilsArcTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("A half arc above 180 degrees covers the complete circle"),
	         UBertaMathUtils::IsAngleInArc(180.0f, 0.0f, 270.0f));
	TestFalse(TEXT("A negative half arc becomes zero-width"),
	          UBertaMathUtils::IsAngleInArc(1.0f, 0.0f, -5.0f));
	return true;
}

#endif
