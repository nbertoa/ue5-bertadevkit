#pragma once

#include "BehaviorTree/BTService.h"
#include "GameplayTagContainer.h"

#include "BertaBTService_TraceGameplayTags.generated.h"

class UAbilitySystemComponent;

/** Logs configured Gameplay Tag count transitions while its branch is relevant, without ticking. */
UCLASS()
class BERTADEVKIT_API UBertaBTService_TraceGameplayTags : public UBTService
{
	GENERATED_BODY()

public:
	UBertaBTService_TraceGameplayTags(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	FGameplayTagContainer ObservedTags;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FString Label;

private:
	struct FGameplayTagEventRegistration
	{
		FGameplayTag Tag;
		FDelegateHandle Handle;
	};

	void HandleGameplayTagChanged(FGameplayTag Tag, int32 NewCount);
	void UnregisterGameplayTagEvents();

	TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystemComponent;
	TArray<FGameplayTagEventRegistration> Registrations;
	bool bIsObserving = false;
};
