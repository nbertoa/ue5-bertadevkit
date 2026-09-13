#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"

#include "BertaBTTask_SendGameplayEvent.generated.h"

class UBehaviorTree;

/** Sends one GAS Gameplay Event to the controlled Pawn. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_SendGameplayEvent : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_SendGameplayEvent(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Event", meta = (GameplayTagFilter = "GameplayEventTagsCategory"))
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category = "Gameplay Event")
	float EventMagnitude = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Gameplay Event")
	FBlackboardKeySelector TargetActorKey;
};
