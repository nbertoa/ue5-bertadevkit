#pragma once

#include "BehaviorTree/BTTaskNode.h"

#include "BertaBTTask_CancelGameplayAbility.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
class UGameplayAbility;

/** Cancels active executions of one exact granted ability spec. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_CancelGameplayAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_CancelGameplayAbility(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
};
