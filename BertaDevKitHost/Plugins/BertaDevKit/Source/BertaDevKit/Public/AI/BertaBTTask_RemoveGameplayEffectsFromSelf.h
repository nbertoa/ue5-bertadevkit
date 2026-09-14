#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "GameplayEffect.h"

#include "BertaBTTask_RemoveGameplayEffectsFromSelf.generated.h"

/** Removes active Gameplay Effects matching a non-empty query from the controlled Pawn ASC. */
UCLASS()
class BERTADEVKIT_API UBertaBTTask_RemoveGameplayEffectsFromSelf : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBertaBTTask_RemoveGameplayEffectsFromSelf(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	FGameplayEffectQuery GameplayEffectQuery;
};
