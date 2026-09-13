#include "AI/BertaBTTask_ActivateGameplayAbilityWithTarget.h"

#include "AIController.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

UBertaBTTask_ActivateGameplayAbilityWithTarget::UBertaBTTask_ActivateGameplayAbilityWithTarget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Activate Gameplay Ability With Target");
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTTask_ActivateGameplayAbilityWithTarget::InitializeFromAsset(UBehaviorTree& Asset)
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

EBTNodeResult::Type UBertaBTTask_ActivateGameplayAbilityWithTarget::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard && TargetActorKey.IsSet()
		? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName))
		: nullptr;
	if (!AbilityClass || !EventTag.IsValid() || !AbilitySystemComponent || !ControlledPawn || !TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass);
	FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();
	if (!AbilitySpec || !ActorInfo)
	{
		return EBTNodeResult::Failed;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = ControlledPawn;
	Payload.Target = TargetActor;
	Payload.EventMagnitude = EventMagnitude;
	const bool bTriggered = AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
		AbilitySpec->Handle,
		ActorInfo,
		EventTag,
		&Payload,
		*AbilitySystemComponent);
	return bTriggered ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBertaBTTask_ActivateGameplayAbilityWithTarget::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nAbility: %s\nTarget: %s\nEvent: %s\nMagnitude: %g"),
		*Super::GetStaticDescription(),
		*GetNameSafe(AbilityClass),
		*TargetActorKey.SelectedKeyName.ToString(),
		*EventTag.ToString(),
		EventMagnitude);
}
