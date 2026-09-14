#include "AI/BertaBTTask_RemoveGameplayEffectsFromTarget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Actor.h"

UBertaBTTask_RemoveGameplayEffectsFromTarget::UBertaBTTask_RemoveGameplayEffectsFromTarget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Remove Gameplay Effects From Target");
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey),
		AActor::StaticClass());
}

void UBertaBTTask_RemoveGameplayEffectsFromTarget::InitializeFromAsset(UBehaviorTree& Asset)
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

EBTNodeResult::Type UBertaBTTask_RemoveGameplayEffectsFromTarget::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (!TargetActorKey.IsSet() || GameplayEffectQuery.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard
		? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName))
		: nullptr;
	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
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

FString UBertaBTTask_RemoveGameplayEffectsFromTarget::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\nTarget: %s\nGameplay Effect Query: %s"),
		*Super::GetStaticDescription(),
		*TargetActorKey.SelectedKeyName.ToString(),
		GameplayEffectQuery.IsEmpty() ? TEXT("None") : TEXT("Configured"));
}
