#pragma once

#include "BehaviorTree/BTTaskNode.h"

#include "BertaBTTask_DebugGASState.generated.h"

/** Logs one controlled-Pawn GAS snapshot and completes immediately. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_DebugGASState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_DebugGASState(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FString Label;
};
