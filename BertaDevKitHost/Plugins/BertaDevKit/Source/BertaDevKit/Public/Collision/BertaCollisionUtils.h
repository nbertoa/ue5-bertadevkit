// BertaCollisionUtils.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaCollisionUtils.generated.h"

class UPrimitiveComponent;

/** Blueprint conveniences for inspecting collision configuration. */
UCLASS()
class BERTADEVKIT_API UBertaCollisionUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns a human-readable summary of the configured collision relationship between two components. */
	UFUNCTION(BlueprintPure,
		Category = "BertaDevKit|Collision|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get Collision Pair Debug Summary"))
	static FString GetCollisionPairDebugSummary(
		const UPrimitiveComponent* A,
		const UPrimitiveComponent* B);
};
