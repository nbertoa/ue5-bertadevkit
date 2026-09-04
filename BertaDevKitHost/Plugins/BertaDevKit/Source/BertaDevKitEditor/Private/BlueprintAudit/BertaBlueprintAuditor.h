#pragma once

#include "AssetRegistry/AssetData.h"

/** Read-only, conservative static Blueprint audit. */
class FBertaBlueprintAuditor
{
public:
	static void Audit(const TArray<FAssetData>& Assets);
};
