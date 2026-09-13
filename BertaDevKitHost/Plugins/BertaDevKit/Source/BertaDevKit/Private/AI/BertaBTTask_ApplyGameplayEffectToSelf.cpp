#include "AI/BertaBTTask_ApplyGameplayEffectToSelf.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ActiveGameplayEffectHandle.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

UBertaBTTask_ApplyGameplayEffectToSelf::UBertaBTTask_ApplyGameplayEffectToSelf(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Apply Gameplay Effect To Self");
}

EBTNodeResult::Type UBertaBTTask_ApplyGameplayEffectToSelf::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (!GameplayEffectClass || !FMath::IsFinite(Level))
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(OwnerComp);
	if (!AbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		GameplayEffectClass,
		Level,
		AbilitySystemComponent->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	const FActiveGameplayEffectHandle ActiveHandle =
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return ActiveHandle.WasSuccessfullyApplied() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBertaBTTask_ApplyGameplayEffectToSelf::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nEffect: %s\nLevel: %g"),
		*Super::GetStaticDescription(),
		*GetNameSafe(GameplayEffectClass),
		Level);
}

UAbilitySystemComponent* UBertaBTTask_ApplyGameplayEffectToSelf::ResolveAbilitySystemComponent(
	const UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
}
