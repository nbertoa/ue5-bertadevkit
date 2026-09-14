#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

#include "BertaBTTask_ApplyGameplayEffectToTarget.generated.h"

class UBehaviorTree;
class UGameplayEffect;

/** Applies one Gameplay Effect from the controlled Pawn ASC to a Blackboard-selected Actor ASC. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_ApplyGameplayEffectToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_ApplyGameplayEffectToTarget(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	float Level = 1.0f;
};
