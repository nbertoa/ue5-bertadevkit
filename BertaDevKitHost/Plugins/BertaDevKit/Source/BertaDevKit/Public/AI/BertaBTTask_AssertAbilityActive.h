#pragma once

#include "BehaviorTree/BTTaskNode.h"

#include "BertaBTTask_AssertAbilityActive.generated.h"

class UGameplayAbility;

/** Validates a controlled-Pawn Gameplay Ability active-state expectation for R&D diagnostics. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_AssertAbilityActive : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_AssertAbilityActive(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, Category = "Assertion")
	bool bExpectedActive = true;

	UPROPERTY(EditAnywhere, Category = "Assertion")
	FString Label;
};
