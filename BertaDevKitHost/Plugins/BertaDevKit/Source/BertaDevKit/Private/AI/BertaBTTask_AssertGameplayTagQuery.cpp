#include "AI/BertaBTTask_AssertGameplayTagQuery.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Debug/BertaDebugLog.h"
#include "GameFramework/Pawn.h"

UBertaBTTask_AssertGameplayTagQuery::UBertaBTTask_AssertGameplayTagQuery(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Assert Gameplay Tag Query");
}

EBTNodeResult::Type UBertaBTTask_AssertGameplayTagQuery::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const FString LabelPrefix = Label.IsEmpty() ? FString() : FString::Printf(TEXT("[%s] "), *Label);
	if (GameplayTagQuery.IsEmpty())
	{
		UE_LOG(LogBertaDebug, Warning, TEXT("%sGAS assertion failed: Gameplay Tag Query is empty."), *LabelPrefix);
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

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
	const bool bActualMatch = GameplayTagQuery.Matches(OwnedTags);
	if (bActualMatch != bExpectedMatch)
	{
		UE_LOG(
			LogBertaDebug,
			Warning,
			TEXT("%sGAS assertion failed: expected query match=%s, actual=%s, query=%s."),
			*LabelPrefix,
			bExpectedMatch ? TEXT("true") : TEXT("false"),
			bActualMatch ? TEXT("true") : TEXT("false"),
			*GameplayTagQuery.GetDescription());
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_AssertGameplayTagQuery::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nExpected: %s\nGameplay Tag Query:\n%s\nLabel: %s"),
		*Super::GetStaticDescription(),
		bExpectedMatch ? TEXT("Matches") : TEXT("Does Not Match"),
		GameplayTagQuery.IsEmpty() ? TEXT("None") : *GameplayTagQuery.GetDescription(),
		Label.IsEmpty() ? TEXT("None") : *Label);
}
