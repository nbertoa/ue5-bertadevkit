#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaBlackboardDebugUtils.generated.h"

class UBlackboardComponent;

/** Read-only Blackboard snapshots backed by UE's native key descriptions. */
UCLASS()
class BERTADEVKIT_API UBertaBlackboardDebugUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|AI|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get Blackboard Debug Summary", ReturnDisplayName = "Success"))
	static bool GetDebugSummary(const UBlackboardComponent* Blackboard, FString& OutSummary);

	/** Shared native formatter used by event-driven trace nodes. */
	static bool GetKeyValueDescription(
		const UBlackboardComponent* Blackboard,
		FName KeyName,
		FString& OutDescription);
};
