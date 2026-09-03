#pragma once

#include "AssetRegistry/AssetData.h"

class FBertaContentBrowserMenu
{
public:
	void Register();
	void Unregister();

	static void FilterProjectAssets(const TArray<FAssetData>& InAssets, TArray<FAssetData>& OutAssets);
	static void FilterAssetsInProjectFolders(const TArray<FString>& InFolders, const TArray<FAssetData>& InAssets, TArray<FAssetData>& OutAssets);

private:
	static void GatherAssetsInProjectFolders(const TArray<FString>& InFolders, TArray<FAssetData>& OutAssets);
};
