#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"

#include "BertaBTTask_ActivateGameplayAbilityWithTarget.generated.h"

class UBehaviorTree;
class UGameplayAbility;

/** Triggers one granted ability using a Gameplay Event with a Blackboard Actor target. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_ActivateGameplayAbilityWithTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_ActivateGameplayAbilityWithTarget(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Gameplay Event", meta = (GameplayTagFilter = "GameplayEventTagsCategory"))
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category = "Gameplay Event")
	float EventMagnitude = 0.0f;
};
