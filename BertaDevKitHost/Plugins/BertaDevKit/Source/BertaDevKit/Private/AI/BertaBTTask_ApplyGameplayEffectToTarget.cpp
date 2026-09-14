#include "AI/BertaBTTask_ApplyGameplayEffectToTarget.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ActiveGameplayEffectHandle.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

UBertaBTTask_ApplyGameplayEffectToTarget::UBertaBTTask_ApplyGameplayEffectToTarget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Apply Gameplay Effect To Target");
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTTask_ApplyGameplayEffectToTarget::InitializeFromAsset(UBehaviorTree& Asset)
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

EBTNodeResult::Type UBertaBTTask_ApplyGameplayEffectToTarget::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (!TargetActorKey.IsSet() || !GameplayEffectClass || !FMath::IsFinite(Level))
	{
		return EBTNodeResult::Failed;
	}

	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	UAbilitySystemComponent* SourceAbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ControlledPawn);
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard
		? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName))
		: nullptr;
	UAbilitySystemComponent* TargetAbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!SourceAbilitySystemComponent || !TargetAbilitySystemComponent)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayEffectSpecHandle SpecHandle = SourceAbilitySystemComponent->MakeOutgoingSpec(
		GameplayEffectClass,
		Level,
		SourceAbilitySystemComponent->MakeEffectContext());
	if (!SpecHandle.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	const FActiveGameplayEffectHandle ActiveHandle = SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(),
		TargetAbilitySystemComponent);
	return ActiveHandle.WasSuccessfullyApplied() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBertaBTTask_ApplyGameplayEffectToTarget::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nTarget: %s\nEffect: %s\nLevel: %g"),
		*Super::GetStaticDescription(),
		*TargetActorKey.SelectedKeyName.ToString(),
		*GetNameSafe(GameplayEffectClass),
		Level);
}
