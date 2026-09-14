#include "AI/BertaBTTask_RemoveGameplayEffectsFromSelf.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

UBertaBTTask_RemoveGameplayEffectsFromSelf::UBertaBTTask_RemoveGameplayEffectsFromSelf(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Remove Gameplay Effects From Self");
}

EBTNodeResult::Type UBertaBTTask_RemoveGameplayEffectsFromSelf::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (GameplayEffectQuery.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
	if (!AbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	if (AbilitySystemComponent->GetActiveEffects(GameplayEffectQuery).IsEmpty())
	{
		return EBTNodeResult::Succeeded;
	}

	return AbilitySystemComponent->RemoveActiveEffects(GameplayEffectQuery) > 0
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}

FString UBertaBTTask_RemoveGameplayEffectsFromSelf::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nGameplay Effect Query: %s"),
		*Super::GetStaticDescription(),
		GameplayEffectQuery.IsEmpty() ? TEXT("None") : TEXT("Configured"));
}
