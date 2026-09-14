#pragma once

#include "BehaviorTree/BTService.h"

#include "BertaBTService_TraceBlackboardChanges.generated.h"

class UBlackboardComponent;

/** Logs selected Blackboard changes while its branch is relevant, without ticking. */
UCLASS()
class BERTADEVKIT_API UBertaBTService_TraceBlackboardChanges : public UBTService
{
	GENERATED_BODY()

public:
	UBertaBTService_TraceBlackboardChanges(const FObjectInitializer& ObjectInitializer);

	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp) override;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FString Label;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	TArray<FName> Keys;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	bool bTraceAllKeysWhenEmpty = true;

private:
	struct FBlackboardObserverRegistration
	{
		FBlackboard::FKey KeyId = FBlackboard::InvalidKey;
		FName KeyName;
		FDelegateHandle Handle;
	};

	EBlackboardNotificationResult HandleBlackboardChange(
		const UBlackboardComponent& Blackboard,
		FBlackboard::FKey ChangedKeyId);
	void UnregisterObservers();

	TWeakObjectPtr<UBlackboardComponent> ObservedBlackboard;
	TArray<FBlackboardObserverRegistration> Registrations;
};
