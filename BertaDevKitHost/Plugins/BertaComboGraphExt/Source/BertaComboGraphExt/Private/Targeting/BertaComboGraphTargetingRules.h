#pragma once

#include "Engine/HitResult.h"

namespace BertaComboGraphTargetingRules
{
	inline void StableUniqueActorHits(const TArray<FHitResult>& Hits, TArray<FHitResult>& OutUniqueHits)
	{
		TSet<TWeakObjectPtr<AActor>> SeenActors;
		OutUniqueHits.Reset();
		OutUniqueHits.Reserve(Hits.Num());
		for (const FHitResult& Hit : Hits)
		{
			AActor* Target = Hit.GetActor();
			if (!Target || SeenActors.Contains(Target))
			{
				continue;
			}
			SeenActors.Add(Target);
			OutUniqueHits.Add(Hit);
		}
	}
}
