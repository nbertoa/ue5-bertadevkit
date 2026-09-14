#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaGameplayTagDebugUtils.generated.h"

class AActor;

/** Read-only, deterministic Gameplay Tag diagnostics for Runtime R&D workflows. */
UCLASS()
class BERTADEVKIT_API UBertaGameplayTagDebugUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|Gameplay Tags|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get Gameplay Tag Container Summary"))
	static void GetGameplayTagContainerSummary(const FGameplayTagContainer& Tags, FString& OutSummary);

	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|Gameplay Tags|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get Gameplay Tag Query Summary"))
	static void GetGameplayTagQuerySummary(const FGameplayTagQuery& Query, FString& OutSummary);

	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|Gameplay Tags|Debug",
		meta = (DevelopmentOnly, DisplayName = "Diff Gameplay Tag Containers"))
	static void DiffGameplayTagContainers(
		const FGameplayTagContainer& Before,
		const FGameplayTagContainer& After,
		TArray<FGameplayTag>& OutAdded,
		TArray<FGameplayTag>& OutRemoved);

	UFUNCTION(
		BlueprintCallable,
		Category = "BertaDevKit|Gameplay Tags|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get Actor Gameplay Tag Summary", ReturnDisplayName = "Success"))
	static bool GetActorGameplayTagSummary(AActor* Actor, FString& OutSummary);
};
