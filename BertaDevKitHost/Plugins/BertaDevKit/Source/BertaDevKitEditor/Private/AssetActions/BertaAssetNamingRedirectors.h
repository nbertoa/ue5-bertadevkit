#pragma once

#include "AssetActions/BertaAssetNamingBatch.h"

class UObjectRedirector;

namespace BertaAssetNamingRedirectors
{
	/** Finds redirectors only at the source object paths recorded by this rename batch. */
	void FindAtBatchSourcePaths(const TArray<FBertaAssetNamingBatchCandidate>& BatchCandidates, TArray<UObjectRedirector*>& OutRedirectors);
}
