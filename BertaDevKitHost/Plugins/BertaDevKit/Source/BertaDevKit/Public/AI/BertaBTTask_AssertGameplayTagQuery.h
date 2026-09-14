#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"

#include "BertaBTTask_AssertGameplayTagQuery.generated.h"

/** Validates a controlled-Pawn Gameplay Tag Query expectation for R&D diagnostics. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_AssertGameplayTagQuery : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_AssertGameplayTagQuery(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	FGameplayTagQuery GameplayTagQuery;

	UPROPERTY(EditAnywhere, Category = "Assertion")
	bool bExpectedMatch = true;

	UPROPERTY(EditAnywhere, Category = "Assertion")
	FString Label;
};
