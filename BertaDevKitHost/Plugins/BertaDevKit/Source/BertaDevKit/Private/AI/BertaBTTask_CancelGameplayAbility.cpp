#include "AI/BertaBTTask_CancelGameplayAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

UBertaBTTask_CancelGameplayAbility::UBertaBTTask_CancelGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Cancel Gameplay Ability");
}

EBTNodeResult::Type UBertaBTTask_CancelGameplayAbility::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (!AbilityClass)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass);
	if (!AbilitySpec)
	{
		return EBTNodeResult::Failed;
	}

	if (AbilitySpec->IsActive())
	{
		AbilitySystemComponent->CancelAbilityHandle(AbilitySpec->Handle);
	}

	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_CancelGameplayAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nAbility: %s"), *Super::GetStaticDescription(), *GetNameSafe(AbilityClass));
}

UAbilitySystemComponent* UBertaBTTask_CancelGameplayAbility::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}
