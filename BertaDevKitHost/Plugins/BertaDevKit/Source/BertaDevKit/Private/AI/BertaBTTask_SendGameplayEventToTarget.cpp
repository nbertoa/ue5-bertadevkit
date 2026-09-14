#include "AI/BertaBTTask_SendGameplayEventToTarget.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

UBertaBTTask_SendGameplayEventToTarget::UBertaBTTask_SendGameplayEventToTarget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Send Gameplay Event To Target");
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTTask_SendGameplayEventToTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (const UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BlackboardAsset);
	}
	else
	{
		TargetActorKey.InvalidateResolvedKey();
	}
}

EBTNodeResult::Type UBertaBTTask_SendGameplayEventToTarget::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard && TargetActorKey.IsSet()
		? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName))
		: nullptr;
	if (!EventTag.IsValid() || !ControlledPawn || !TargetActor ||
		!UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = ControlledPawn;
	Payload.Target = TargetActor;
	Payload.EventMagnitude = EventMagnitude;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventTag, Payload);
	return EBTNodeResult::Succeeded;
}

FString UBertaBTTask_SendGameplayEventToTarget::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nTarget: %s\nEvent: %s\nMagnitude: %g"),
		*Super::GetStaticDescription(),
		*TargetActorKey.SelectedKeyName.ToString(),
		*EventTag.ToString(),
		EventMagnitude);
}
