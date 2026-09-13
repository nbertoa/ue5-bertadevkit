#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaGASDebugUtils.generated.h"

class AActor;

/** Compact Runtime GAS snapshots intended for development diagnostics. */
UCLASS()
class BERTADEVKIT_API UBertaGASDebugUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|GAS|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get GAS Debug Summary", ReturnDisplayName = "Success"))
	static bool GetDebugSummary(AActor* Actor, FString& OutSummary);
};
