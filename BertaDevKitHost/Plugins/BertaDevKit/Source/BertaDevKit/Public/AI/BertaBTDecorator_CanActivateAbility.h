#pragma once

#include "BehaviorTree/BTDecorator.h"

#include "BertaBTDecorator_CanActivateAbility.generated.h"

class UGameplayAbility;

/** Tests whether an exact granted ability reports that it can activate now. */
UCLASS()
class BERTADEVKIT_API UBertaBTDecorator_CanActivateAbility : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBertaBTDecorator_CanActivateAbility(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;
};
