#include "AssetInsights/BertaAssetInsights.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetDataToken.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	const FName AssetInsightsLogName(TEXT("BertaDevKitAssetInsights"));

	FString FormatBytes(const int64 Bytes)
	{
		return Bytes < 0 ? TEXT("Unavailable") : FString::Printf(TEXT("%.2f MiB"), static_cast<double>(Bytes) / (1024.0 * 1024.0));
	}

	void AddAssetMessage(FMessageLog& MessageLog, const FAssetData& Asset, const FString& Text)
	{
		TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
		Message->AddToken(FAssetDataToken::Create(Asset));
		Message->AddToken(FTextToken::Create(FText::FromString(TEXT("  ") + Text)));
		MessageLog.AddMessage(Message);
	}

	void AddInfoMessage(FMessageLog& MessageLog, const FString& Text)
	{
		MessageLog.AddMessage(FTokenizedMessage::Create(EMessageSeverity::Info, FText::FromString(Text)));
	}

	bool IsProjectAssetForInsights(const FAssetData& Asset)
	{
		const FString Path = Asset.PackagePath.ToString();
		return Path == TEXT("/Game") || Path.StartsWith(TEXT("/Game/"));
	}

	bool IsLoadedPackageDirty(const FAssetData& Asset)
	{
		if (UObject* LoadedAsset = Asset.FastGetAsset(false))
		{
			return LoadedAsset->GetOutermost()->IsDirty();
		}
		return false;
	}

	FBertaTextureInsightsMetrics GetTextureMetrics(const UTexture2D& Texture)
	{
		FBertaTextureInsightsMetrics Metrics;
		Metrics.MaxSourceDimension = FMath::Max(Texture.Source.GetSizeX(), Texture.Source.GetSizeY());
		Metrics.MaximumTextureSize = Texture.GetMaxTextureSize();
		Metrics.bNeverStream = Texture.NeverStream;
		Metrics.bNoMipmaps = Texture.MipGenSettings == TMGS_NoMipmaps;
		Metrics.bSuppressStreamingReviews = Texture.LODGroup == TEXTUREGROUP_UI || Texture.LODGroup == TEXTUREGROUP_Pixels2D;
		return Metrics;
	}

	void ReportTexture(FMessageLog& MessageLog, const FAssetData& Asset, UTexture2D& Texture, int32& InOutReviewFindings)
	{
		const FBertaTextureInsightsMetrics Metrics = GetTextureMetrics(Texture);
		int32 BuiltSizeX = 0, BuiltSizeY = 0, BuiltSizeZ = 0;
		Texture.GetBuiltTextureSize(static_cast<const ITargetPlatform*>(nullptr), BuiltSizeX, BuiltSizeY, BuiltSizeZ);
		const uint32 AllMipsMemory = Texture.CalcTextureMemorySizeEnum(TMC_AllMips);
		AddAssetMessage(MessageLog, Asset, FString::Printf(TEXT("[Texture2D] Source: %dx%d | Built (current editor context): %dx%d | Maximum Texture Size: %d | Texture Group: %s | LOD Bias: %d | Mip Generation: %s | Never Stream: %s | Virtual Texture Streaming: %s | Current-platform all-mips texture memory estimate: %s."), Texture.Source.GetSizeX(), Texture.Source.GetSizeY(), BuiltSizeX, BuiltSizeY, Texture.GetMaxTextureSize(), UTexture::GetTextureGroupString(Texture.LODGroup), Texture.LODBias, UTexture::GetMipGenSettingsString(Texture.MipGenSettings), Texture.NeverStream ? TEXT("true") : TEXT("false"), Texture.VirtualTextureStreaming ? TEXT("true") : TEXT("false"), AllMipsMemory > 0 ? *FormatBytes(AllMipsMemory) : TEXT("Unavailable")));

		TArray<EBertaAssetInsightsReview> Reviews;
		BertaAssetInsightsRules::EvaluateTextureReviews(Metrics, Reviews);
		for (const EBertaAssetInsightsReview Review : Reviews)
		{
			++InOutReviewFindings;
			switch (Review)
			{
			case EBertaAssetInsightsReview::TextureResolution:
				AddAssetMessage(MessageLog, Asset, FString::Printf(TEXT("[Review] Texture Resolution: Source is %dx%d and per-asset Maximum Texture Size is unrestricted. Texture group/platform settings may still clamp it; review whether full source resolution is needed."), Texture.Source.GetSizeX(), Texture.Source.GetSizeY()));
				break;
			case EBertaAssetInsightsReview::NeverStream:
				AddAssetMessage(MessageLog, Asset, TEXT("[Review] Never Stream: This large texture can increase resident texture-memory pressure; review whether Never Stream is intentional."));
				break;
			case EBertaAssetInsightsReview::MipGeneration:
				AddAssetMessage(MessageLog, Asset, TEXT("[Review] Mip Generation: Review whether a large texture intentionally needs no mip chain."));
				break;
			default: break;
			}
		}
	}

	void ReportStaticMesh(FMessageLog& MessageLog, const FAssetData& Asset, UStaticMesh& StaticMesh, int32& InOutReviewFindings)
	{
		FBertaStaticMeshInsightsMetrics Metrics;
		Metrics.bNaniteEnabled = StaticMesh.IsNaniteEnabled();
		Metrics.LODCount = StaticMesh.GetNumLODs();
		Metrics.LOD0TriangleCount = Metrics.LODCount > 0 ? StaticMesh.GetNumTriangles(0) : 0;
		FString LODMetrics;
		for (int32 LODIndex = 0; LODIndex < Metrics.LODCount; ++LODIndex)
		{
			LODMetrics += FString::Printf(TEXT("%sLOD%d: %d triangles / %d vertices"), LODIndex == 0 ? TEXT("") : TEXT("; "), LODIndex, StaticMesh.GetNumTriangles(LODIndex), StaticMesh.GetNumVertices(LODIndex));
		}
		const int32 UVChannels = Metrics.LODCount > 0 ? StaticMesh.GetNumUVChannels(0) : 0;
		const SIZE_T ResourceSize = StaticMesh.GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);
		AddAssetMessage(MessageLog, Asset, FString::Printf(TEXT("[StaticMesh] Nanite: %s | LODs: %d | %s | LOD0 UV Channels: %d | Material Slots: %d | Estimated Editor Resource Size: %s."), Metrics.bNaniteEnabled ? TEXT("enabled") : TEXT("disabled"), Metrics.LODCount, *LODMetrics, UVChannels, StaticMesh.GetStaticMaterials().Num(), *FormatBytes(static_cast<int64>(ResourceSize))));

		TArray<EBertaAssetInsightsReview> Reviews;
		BertaAssetInsightsRules::EvaluateStaticMeshReviews(Metrics, Reviews);
		for (const EBertaAssetInsightsReview Review : Reviews)
		{
			++InOutReviewFindings;
			if (Review == EBertaAssetInsightsReview::LODStrategy)
			{
				AddAssetMessage(MessageLog, Asset, TEXT("[Review] LOD Strategy: This non-Nanite mesh has one LOD and at least 100,000 LOD0 triangles. Review traditional LODs or Nanite where appropriate."));
			}
			else if (Review == EBertaAssetInsightsReview::HighGeometryFootprint)
			{
				AddAssetMessage(MessageLog, Asset, TEXT("[Review] High Geometry Footprint: This non-Nanite mesh has at least 500,000 LOD0 triangles. Review source geometry, traditional LOD strategy, and Nanite suitability."));
			}
		}
	}
}

void BertaAssetInsightsRules::EvaluateTextureReviews(const FBertaTextureInsightsMetrics& Metrics, TArray<EBertaAssetInsightsReview>& OutReviews)
{
	OutReviews.Reset();
	if (Metrics.MaxSourceDimension >= 4096 && Metrics.MaximumTextureSize == 0)
	{
		OutReviews.Add(EBertaAssetInsightsReview::TextureResolution);
	}
	if (!Metrics.bSuppressStreamingReviews && Metrics.MaxSourceDimension >= 2048 && Metrics.bNeverStream)
	{
		OutReviews.Add(EBertaAssetInsightsReview::NeverStream);
	}
	if (!Metrics.bSuppressStreamingReviews && Metrics.MaxSourceDimension >= 2048 && Metrics.bNoMipmaps)
	{
		OutReviews.Add(EBertaAssetInsightsReview::MipGeneration);
	}
}

void BertaAssetInsightsRules::EvaluateStaticMeshReviews(const FBertaStaticMeshInsightsMetrics& Metrics, TArray<EBertaAssetInsightsReview>& OutReviews)
{
	OutReviews.Reset();
	if (!Metrics.bNaniteEnabled && Metrics.LODCount == 1 && Metrics.LOD0TriangleCount >= 100000)
	{
		OutReviews.Add(EBertaAssetInsightsReview::LODStrategy);
	}
	if (!Metrics.bNaniteEnabled && Metrics.LOD0TriangleCount >= 500000)
	{
		OutReviews.Add(EBertaAssetInsightsReview::HighGeometryFootprint);
	}
}

void FBertaAssetInsights::Analyze(const TArray<FAssetData>& Assets)
{
	TArray<FAssetData> ProjectAssets;
	for (const FAssetData& Asset : Assets)
	{
		if (IsProjectAssetForInsights(Asset))
		{
			ProjectAssets.AddUnique(Asset);
		}
	}
	if (ProjectAssets.IsEmpty())
	{
		UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetInsights] No /Game assets selected."));
		return;
	}

	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	ProjectAssets.Sort([&Registry](const FAssetData& Left, const FAssetData& Right)
	{
		FAssetPackageData LeftData, RightData;
		const bool bHasLeft = Registry.TryGetAssetPackageData(Left.PackageName, LeftData) == UE::AssetRegistry::EExists::Exists;
		const bool bHasRight = Registry.TryGetAssetPackageData(Right.PackageName, RightData) == UE::AssetRegistry::EExists::Exists;
		return (bHasLeft ? LeftData.DiskSize : -1) > (bHasRight ? RightData.DiskSize : -1);
	});

	FMessageLog MessageLog(AssetInsightsLogName);
	MessageLog.NewPage(FText::Format(NSLOCTEXT("BertaDevKit", "AssetInsightsPage", "Asset Insights {0}"), FText::AsDateTime(FDateTime::Now())));
	AddInfoMessage(MessageLog, TEXT("[Info] Saved Package Size is saved package data, not cooked/package size. Use Unreal Size Map for transitive dependency/editor footprint, Reference Viewer for reference trees, and Asset Audit for native detailed asset metadata."));
	int32 TextureCount = 0, StaticMeshCount = 0, GeneralOnlyCount = 0, LoadFailures = 0, ReviewFindings = 0, PackageSizeUnavailable = 0;
	TMap<FName, int64> UniquePackageSizes;
	for (const FAssetData& Asset : ProjectAssets)
	{
		FAssetPackageData PackageData;
		const bool bHasPackageData = Registry.TryGetAssetPackageData(Asset.PackageName, PackageData) == UE::AssetRegistry::EExists::Exists;
		if (bHasPackageData)
		{
			UniquePackageSizes.FindOrAdd(Asset.PackageName) = PackageData.DiskSize;
		}
		else
		{
			++PackageSizeUnavailable;
		}
		TArray<FName> Dependencies, Referencers;
		const bool bDependenciesAvailable = Registry.GetDependencies(Asset.PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::Package);
		const bool bReferencersAvailable = Registry.GetReferencers(Asset.PackageName, Referencers, UE::AssetRegistry::EDependencyCategory::Package);
		AddAssetMessage(MessageLog, Asset, FString::Printf(TEXT("[Info] %s | Saved Package Size: %s | Direct on-disk package dependencies: %s | Direct on-disk package referencers: %s.%s"), *Asset.AssetClassPath.ToString(), *FormatBytes(bHasPackageData ? PackageData.DiskSize : -1), bDependenciesAvailable ? *LexToString(Dependencies.Num()) : TEXT("Unavailable"), bReferencersAvailable ? *LexToString(Referencers.Num()) : TEXT("Unavailable"), IsLoadedPackageDirty(Asset) ? TEXT(" Saved size may be stale because the package has unsaved changes.") : TEXT("")));

		if (Asset.AssetClassPath == UTexture2D::StaticClass()->GetClassPathName())
		{
			if (UTexture2D* Texture = Cast<UTexture2D>(Asset.GetAsset())) { ++TextureCount; ReportTexture(MessageLog, Asset, *Texture, ReviewFindings); } else { ++LoadFailures; UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetInsights] Failed to load Texture2D: %s"), *Asset.GetSoftObjectPath().ToString()); }
		}
		else if (Asset.AssetClassPath == UStaticMesh::StaticClass()->GetClassPathName())
		{
			if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset.GetAsset())) { ++StaticMeshCount; ReportStaticMesh(MessageLog, Asset, *StaticMesh, ReviewFindings); } else { ++LoadFailures; UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[AssetInsights] Failed to load StaticMesh: %s"), *Asset.GetSoftObjectPath().ToString()); }
		}
		else
		{
			++GeneralOnlyCount;
		}
	}
	int64 UniquePackageTotal = 0;
	for (const TPair<FName, int64>& Pair : UniquePackageSizes) { UniquePackageTotal += Pair.Value; }
	const FString Summary = FString::Printf(TEXT("[AssetInsights] Complete: selected assets=%d Texture2D analyzed=%d StaticMesh analyzed=%d general-only assets=%d load failures=%d review findings=%d unavailable Saved Package Sizes=%d unique-package Saved Package Size total (available packages)=%s."), ProjectAssets.Num(), TextureCount, StaticMeshCount, GeneralOnlyCount, LoadFailures, ReviewFindings, PackageSizeUnavailable, *FormatBytes(UniquePackageTotal));
	UE_LOG(LogBertaDevKitEditor, Log, TEXT("%s"), *Summary);
	AddInfoMessage(MessageLog, Summary);
	MessageLog.Flush();
}
