#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaBehaviorTreeDebugUtils.generated.h"

class UBehaviorTreeComponent;

/** Public-API-only Behavior Tree execution snapshots for Runtime diagnostics. */
UCLASS()
class BERTADEVKIT_API UBertaBehaviorTreeDebugUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|AI|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get Behavior Tree Debug Summary", ReturnDisplayName = "Success"))
	static bool GetDebugSummary(
		const UBehaviorTreeComponent* BehaviorTreeComponent,
		FString& OutSummary);
};
