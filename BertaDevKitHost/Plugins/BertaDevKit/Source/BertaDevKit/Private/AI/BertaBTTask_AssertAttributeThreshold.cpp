#include "AI/BertaBTTask_AssertAttributeThreshold.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Debug/BertaDebugLog.h"
#include "GameFramework/Pawn.h"

UBertaBTTask_AssertAttributeThreshold::UBertaBTTask_AssertAttributeThreshold(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Assert Attribute Threshold");
}

EBTNodeResult::Type UBertaBTTask_AssertAttributeThreshold::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const FString LabelPrefix = Label.IsEmpty() ? FString() : FString::Printf(TEXT("[%s] "), *Label);
	if (!Condition.Attribute.IsValid())
	{
		UE_LOG(LogBertaDebug, Warning, TEXT("%sGAS assertion failed: Gameplay Attribute is invalid."), *LabelPrefix);
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
	if (!AbilitySystemComponent->HasAttributeSetForAttribute(Condition.Attribute))
	{
		UE_LOG(
			LogBertaDebug,
			Warning,
			TEXT("%sGAS assertion failed: controlled Pawn ASC does not contain attribute %s."),
			*LabelPrefix,
			*Condition.Attribute.GetName());
		return EBTNodeResult::Failed;
	}

	bool bFound = false;
	const float CurrentValue = AbilitySystemComponent->GetGameplayAttributeValue(Condition.Attribute, bFound);
	if (!bFound)
	{
		UE_LOG(
			LogBertaDebug,
			Warning,
			TEXT("%sGAS assertion failed: could not read attribute %s."),
			*LabelPrefix,
			*Condition.Attribute.GetName());
		return EBTNodeResult::Failed;
	}

	const bool bActualResult = Condition.IsSatisfied(CurrentValue);
	if (bActualResult != bExpectedResult)
	{
		UE_LOG(
			LogBertaDebug,
			Warning,
			TEXT("%sGAS assertion failed: condition '%s', value=%g, expected=%s, actual=%s."),
			*LabelPrefix,
			*Condition.ToString(),
			CurrentValue,
			bExpectedResult ? TEXT("true") : TEXT("false"),
			bActualResult ? TEXT("true") : TEXT("false"));
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_AssertAttributeThreshold::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\n%s\nExpected: %s\nLabel: %s"),
		*Super::GetStaticDescription(),
		*Condition.ToString(),
		bExpectedResult ? TEXT("True") : TEXT("False"),
		Label.IsEmpty() ? TEXT("None") : *Label);
}
