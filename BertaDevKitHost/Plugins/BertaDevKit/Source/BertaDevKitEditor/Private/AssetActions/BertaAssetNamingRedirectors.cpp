#include "AssetActions/BertaAssetNamingRedirectors.h"

#include "UObject/ObjectRedirector.h"
#include "UObject/UObjectGlobals.h"

void BertaAssetNamingRedirectors::FindAtBatchSourcePaths(const TArray<FBertaAssetNamingBatchCandidate>& BatchCandidates, TArray<UObjectRedirector*>& OutRedirectors)
{
	OutRedirectors.Reset();
	TSet<FString> ProcessedSourcePaths;
	for (const FBertaAssetNamingBatchCandidate& Candidate : BatchCandidates)
	{
		if (Candidate.SourceObjectPath.IsEmpty() || ProcessedSourcePaths.Contains(Candidate.SourceObjectPath))
		{
			continue;
		}
		ProcessedSourcePaths.Add(Candidate.SourceObjectPath);

		if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(StaticLoadObject(UObjectRedirector::StaticClass(), nullptr, *Candidate.SourceObjectPath, nullptr, LOAD_NoWarn | LOAD_NoRedirects)))
		{
			OutRedirectors.Add(Redirector);
		}
	}
}
