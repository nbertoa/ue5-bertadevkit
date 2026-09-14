#include "AI/BertaBTTask_AssertAbilityActive.h"

#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Debug/BertaDebugLog.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

UBertaBTTask_AssertAbilityActive::UBertaBTTask_AssertAbilityActive(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Assert Ability Active");
}

EBTNodeResult::Type UBertaBTTask_AssertAbilityActive::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const FString LabelPrefix = Label.IsEmpty() ? FString() : FString::Printf(TEXT("[%s] "), *Label);
	if (!AbilityClass)
	{
		UE_LOG(LogBertaDebug, Warning, TEXT("%sGAS assertion failed: Gameplay Ability class is invalid."), *LabelPrefix);
		return EBTNodeResult::Failed;
	}

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	const UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogBertaDebug, Warning, TEXT("%sGAS assertion failed: controlled Pawn has no ASC."), *LabelPrefix);
		return EBTNodeResult::Failed;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent
		->FindAbilitySpecFromClass(AbilityClass);
	if (!AbilitySpec)
	{
		UE_LOG(
			LogBertaDebug,
			Warning,
			TEXT("%sGAS assertion failed: ability %s is not granted on the controlled Pawn ASC."),
			*LabelPrefix,
			*GetNameSafe(AbilityClass));
		return EBTNodeResult::Failed;
	}

	const bool bActualActive = AbilitySpec->IsActive();
	if (bActualActive != bExpectedActive)
	{
		UE_LOG(
			LogBertaDebug,
			Warning,
			TEXT("%sGAS assertion failed: ability=%s, expected active=%s, actual=%s."),
			*LabelPrefix,
			*GetNameSafe(AbilityClass),
			bExpectedActive ? TEXT("true") : TEXT("false"),
			bActualActive ? TEXT("true") : TEXT("false"));
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_AssertAbilityActive::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nAbility: %s\nExpected: %s\nLabel: %s"),
		*Super::GetStaticDescription(),
		*GetNameSafe(AbilityClass),
		bExpectedActive ? TEXT("Active") : TEXT("Inactive"),
		Label.IsEmpty() ? TEXT("None") : *Label);
}
