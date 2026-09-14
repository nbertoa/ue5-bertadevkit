#pragma once

#include "AI/BertaGASBehaviorTreeTypes.h"
#include "BehaviorTree/BTTaskNode.h"

#include "BertaBTTask_AssertAttributeThreshold.generated.h"

/** Validates a controlled-Pawn Gameplay Attribute expectation for R&D diagnostics. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_AssertAttributeThreshold : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_AssertAttributeThreshold(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Attribute")
	FBertaGameplayAttributeCondition Condition;

	UPROPERTY(EditAnywhere, Category = "Assertion")
	bool bExpectedResult = true;

	UPROPERTY(EditAnywhere, Category = "Assertion")
	FString Label;
};
