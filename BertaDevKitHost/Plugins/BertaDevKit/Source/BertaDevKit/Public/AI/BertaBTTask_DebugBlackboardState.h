#pragma once

#include "BehaviorTree/BTTaskNode.h"

#include "BertaBTTask_DebugBlackboardState.generated.h"

/** Logs one Blackboard snapshot and completes immediately. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_DebugBlackboardState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_DebugBlackboardState(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FString Label;
};
