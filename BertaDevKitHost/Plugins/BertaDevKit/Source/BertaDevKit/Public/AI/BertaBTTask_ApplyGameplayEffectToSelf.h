#pragma once

#include "BehaviorTree/BTTaskNode.h"

#include "BertaBTTask_ApplyGameplayEffectToSelf.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;
class UGameplayEffect;

/** Builds and applies one Gameplay Effect spec to the controlled Pawn's ASC. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_ApplyGameplayEffectToSelf : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_ApplyGameplayEffectToSelf(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	float Level = 1.0f;

private:
	UAbilitySystemComponent* ResolveAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
};
