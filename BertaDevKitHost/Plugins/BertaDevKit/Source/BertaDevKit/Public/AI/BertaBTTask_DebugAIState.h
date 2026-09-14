#pragma once

#include "BehaviorTree/BTTaskNode.h"

#include "BertaBTTask_DebugAIState.generated.h"

/** Logs one combined AI, Behavior Tree, Blackboard, and GAS snapshot. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_DebugAIState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_DebugAIState(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FString Label;
};
