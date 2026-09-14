#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaAIDebugUtils.generated.h"

class AAIController;

/** Composes one readable Runtime snapshot of an AI controller and its subsystems. */
UCLASS()
class BERTADEVKIT_API UBertaAIDebugUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|AI|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get AI Debug Summary", ReturnDisplayName = "Success"))
	static bool GetDebugSummary(AAIController* AIController, FString& OutSummary);
};
