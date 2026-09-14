#pragma once

#include "Abilities/GSCTargetType.h"

#include "BertaGSCTargetTypes.generated.h"

class UTargetingPreset;

/** Wraps another GSC TargetType, preserving its results while optionally logging and drawing them. */
UCLASS(Blueprintable)
class BERTAGASCOMPANIONEXT_API UBertaGSCTargetType_DebugProxy final : public UGSCTargetType
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaGASCompanionExt|Targeting")
	TSubclassOf<UGSCTargetType> InnerTargetType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaGASCompanionExt|Targeting")
	bool bLogSummary = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaGASCompanionExt|Targeting")
	bool bDebugDraw = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaGASCompanionExt|Targeting", meta = (ClampMin = "0.01"))
	float DebugDrawDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaGASCompanionExt|Targeting", meta = (ClampMin = "1.0"))
	float DebugPointSize = 12.0f;

	virtual void GetTargets_Implementation(
		AActor* TargetingActor,
		FGameplayEventData EventData,
		TArray<FHitResult>& OutHitResults,
		TArray<AActor*>& OutActors) const override;
};

/** Executes a UE 5.8 Gameplay Targeting System preset through its supported immediate request path. */
UCLASS(Blueprintable)
class BERTAGASCOMPANIONEXT_API UBertaGSCTargetType_TargetingPreset final : public UGSCTargetType
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BertaGASCompanionExt|Targeting")
	TObjectPtr<UTargetingPreset> TargetingPreset = nullptr;

	virtual void GetTargets_Implementation(
		AActor* TargetingActor,
		FGameplayEventData EventData,
		TArray<FHitResult>& OutHitResults,
		TArray<AActor*>& OutActors) const override;
};
