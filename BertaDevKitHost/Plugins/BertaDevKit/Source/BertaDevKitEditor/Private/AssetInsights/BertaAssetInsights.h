#pragma once

#include "AssetRegistry/AssetData.h"

enum class EBertaAssetInsightsReview : uint8
{
	TextureResolution,
	NeverStream,
	MipGeneration,
	LODStrategy,
	HighGeometryFootprint
};

struct FBertaTextureInsightsMetrics
{
	int32 MaxSourceDimension = 0;
	int32 MaximumTextureSize = 0;
	bool bNeverStream = false;
	bool bNoMipmaps = false;
	bool bSuppressStreamingReviews = false;
};

struct FBertaStaticMeshInsightsMetrics
{
	bool bNaniteEnabled = false;
	int32 LODCount = 0;
	int32 LOD0TriangleCount = 0;
};

namespace BertaAssetInsightsRules
{
	void EvaluateTextureReviews(const FBertaTextureInsightsMetrics& Metrics, TArray<EBertaAssetInsightsReview>& OutReviews);
	void EvaluateStaticMeshReviews(const FBertaStaticMeshInsightsMetrics& Metrics, TArray<EBertaAssetInsightsReview>& OutReviews);
}

/** Read-only selected-asset footprint metrics and conservative review guidance. */
class FBertaAssetInsights
{
public:
	static void Analyze(const TArray<FAssetData>& Assets);
};
