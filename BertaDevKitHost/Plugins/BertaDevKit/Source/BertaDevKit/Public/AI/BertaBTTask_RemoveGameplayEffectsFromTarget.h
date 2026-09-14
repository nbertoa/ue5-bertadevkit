#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayEffect.h"

#include "BertaBTTask_RemoveGameplayEffectsFromTarget.generated.h"

class UBehaviorTree;

/** Removes active Gameplay Effects matching a non-empty query from a Blackboard-selected Actor ASC. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_RemoveGameplayEffectsFromTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_RemoveGameplayEffectsFromTarget(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	FGameplayEffectQuery GameplayEffectQuery;
};
