#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

#include "BertaBTTask_DebugTargetGASState.generated.h"

class UBehaviorTree;

/** Logs one Blackboard-target GAS snapshot and completes immediately. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_DebugTargetGASState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_DebugTargetGASState(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FString Label;
};
