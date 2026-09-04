#include "AssetInsights/BertaAssetInsights.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaAssetInsightsRulesTest, "BertaDevKit.AssetInsights.ReviewThresholds", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBertaAssetInsightsRulesTest::RunTest(const FString& Parameters)
{
	TArray<EBertaAssetInsightsReview> Reviews;
	FBertaTextureInsightsMetrics Texture { 4096, 0, true, true, false };
	BertaAssetInsightsRules::EvaluateTextureReviews(Texture, Reviews);
	TestEqual(TEXT("4K unrestricted streaming texture has three reviews"), Reviews.Num(), 3);
	Texture.bSuppressStreamingReviews = true;
	BertaAssetInsightsRules::EvaluateTextureReviews(Texture, Reviews);
	TestEqual(TEXT("UI or Pixels2D texture suppresses streaming reviews"), Reviews.Num(), 1);

	FBertaStaticMeshInsightsMetrics Mesh { false, 1, 100000 };
	BertaAssetInsightsRules::EvaluateStaticMeshReviews(Mesh, Reviews);
	TestTrue(TEXT("Non-Nanite 100k one-LOD mesh reviews LOD strategy"), Reviews.Contains(EBertaAssetInsightsReview::LODStrategy));
	Mesh.LOD0TriangleCount = 500000;
	BertaAssetInsightsRules::EvaluateStaticMeshReviews(Mesh, Reviews);
	TestEqual(TEXT("500k non-Nanite mesh has two reviews"), Reviews.Num(), 2);
	Mesh.bNaniteEnabled = true;
	BertaAssetInsightsRules::EvaluateStaticMeshReviews(Mesh, Reviews);
	TestTrue(TEXT("Nanite mesh suppresses traditional LOD reviews"), Reviews.IsEmpty());
	return true;
}

#endif
