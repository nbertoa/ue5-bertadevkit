#include "AI/BertaBTDecorator_CanActivateAbility.h"

#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

UBertaBTDecorator_CanActivateAbility::UBertaBTDecorator_CanActivateAbility(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Can Activate Ability");
}

bool UBertaBTDecorator_CanActivateAbility::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	if (!AbilityClass)
	{
		return false;
	}

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	const UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass)
		: nullptr;
	const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent
		? AbilitySystemComponent->AbilityActorInfo.Get()
		: nullptr;
	if (!AbilitySpec || !ActorInfo)
	{
		return false;
	}

	const UGameplayAbility* Ability = AbilitySpec->GetPrimaryInstance();
	if (!Ability)
	{
		Ability = AbilitySpec->Ability.Get();
	}

	return Ability && Ability->CanActivateAbility(AbilitySpec->Handle, ActorInfo);
}

FString UBertaBTDecorator_CanActivateAbility::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nAbility: %s\nReactivity: Behavior Tree evaluation only"),
		*Super::GetStaticDescription(),
		*GetNameSafe(AbilityClass));
}
