#include "AI/BertaBTTask_SendGameplayEvent.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

UBertaBTTask_SendGameplayEvent::UBertaBTTask_SendGameplayEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Send Gameplay Event");
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
	TargetActorKey.AllowNoneAsValue(true);
}

EBTNodeResult::Type UBertaBTTask_SendGameplayEvent::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!EventTag.IsValid() || !ControlledPawn ||
		!UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn))
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = nullptr;
	if (TargetActorKey.IsSet())
	{
		const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
		TargetActor = BlackboardComponent
			? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName))
			: nullptr;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = ControlledPawn;
	Payload.Target = TargetActor;
	Payload.EventMagnitude = EventMagnitude;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(ControlledPawn, EventTag, Payload);
	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_SendGameplayEvent::GetStaticDescription() const
{
	const FString TargetDescription = TargetActorKey.IsSet()
		? TargetActorKey.SelectedKeyName.ToString()
		: TEXT("None");
	return FString::Printf(
		TEXT("%s\nEvent: %s\nMagnitude: %g\nTarget: %s"),
		*Super::GetStaticDescription(),
		*EventTag.ToString(),
		EventMagnitude,
		*TargetDescription);
}
