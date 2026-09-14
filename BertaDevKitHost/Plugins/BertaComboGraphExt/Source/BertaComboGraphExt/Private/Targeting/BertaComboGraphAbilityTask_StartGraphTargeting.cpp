#include "Targeting/BertaComboGraphAbilityTask_StartGraphTargeting.h"

#include "BertaComboGraphExt.h"
#include "Abilities/GameplayAbility.h"
#include "Targeting/BertaComboGraphTargetingContext.h"
#include "Targeting/BertaComboGraphTargetingNodes.h"
#include "Targeting/BertaComboGraphTargetingRules.h"
#include "TargetingSystem/TargetingPreset.h"
#include "TargetingSystem/TargetingSubsystem.h"

UBertaComboGraphAbilityTask_StartGraphTargeting* UBertaComboGraphAbilityTask_StartGraphTargeting::StartComboGraphWithTargeting(
	UGameplayAbility* OwningAbility,
	UComboGraph* ComboGraph,
	UInputAction* InputAction,
	const bool bBroadcastInternalEvents)
{
	UBertaComboGraphAbilityTask_StartGraphTargeting* Task = NewAbilityTask<UBertaComboGraphAbilityTask_StartGraphTargeting>(OwningAbility);
	if (Task)
	{
		Task->RunningGraph = ComboGraph;
		Task->InitialInput = InputAction;
		Task->bBroadcastInternalEvents = bBroadcastInternalEvents;
	}
	return Task;
}

FComboGraphGameplayEffectContainerSpec UBertaComboGraphAbilityTask_StartGraphTargeting::MakeEffectContainerSpecFromContainer(
	const FComboGraphGameplayEffectContainer& Container,
	const FGameplayTag ContainerTag,
	const FGameplayEventData& EventData,
	const int32 OverrideGameplayLevel)
{
	FComboGraphGameplayEffectContainerSpec Result = Super::MakeEffectContainerSpecFromContainer(Container, ContainerTag, EventData, OverrideGameplayLevel);
	const IBertaComboGraphTargetingProvider* Provider = Cast<IBertaComboGraphTargetingProvider>(CurrentNode);
	const UTargetingPreset* Preset = Provider ? Provider->GetTargetingPresetForEffectEvent(ContainerTag) : nullptr;
	if (!Preset)
	{
		return Result;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	UTargetingSubsystem* Subsystem = Avatar ? UTargetingSubsystem::Get(Avatar->GetWorld()) : nullptr;
	if (!Subsystem)
	{
		UE_LOG(LogBertaComboGraphExt, Warning, TEXT("Targeting preset %s could not run because no TargetingSubsystem was available for %s."), *GetPathNameSafe(Preset), *GetPathNameSafe(Avatar));
		Result.TargetData = FGameplayAbilityTargetDataHandle();
		return Result;
	}

	FTargetingSourceContext SourceContext;
	SourceContext.SourceActor = Avatar;
	AActor* EventInstigator = const_cast<AActor*>(EventData.Instigator.Get());
	SourceContext.InstigatorActor = EventInstigator ? EventInstigator : Avatar;
	UBertaComboGraphTargetingContext* RequestContext = NewObject<UBertaComboGraphTargetingContext>(this);
	RequestContext->CurrentNode = CurrentNode;
	RequestContext->ContainerEventTag = ContainerTag;
	RequestContext->EventData = EventData;
	SourceContext.SourceObject = RequestContext;
	SourceContext.SourceLocation = Avatar->GetActorLocation();
	if (const FHitResult* ContextHit = EventData.ContextHandle.GetHitResult())
	{
		SourceContext.SourceLocation = ContextHit->ImpactPoint;
	}
	else if (const AActor* EventTarget = EventData.Target.Get())
	{
		SourceContext.SourceLocation = EventTarget->GetActorLocation();
	}

	FTargetingRequestHandle Handle = UTargetingSubsystem::MakeTargetRequestHandle(Preset, SourceContext);
	if (!Handle.IsValid())
	{
		UE_LOG(LogBertaComboGraphExt, Warning, TEXT("Targeting preset %s produced an invalid immediate request handle."), *GetPathNameSafe(Preset));
		Result.TargetData = FGameplayAbilityTargetDataHandle();
		return Result;
	}

	Subsystem->ExecuteTargetingRequestWithHandle(Handle);
	TArray<FHitResult> Hits;
	Subsystem->GetTargetingResults(Handle, Hits);
	UTargetingSubsystem::ReleaseTargetRequestHandle(Handle);

	TArray<FHitResult> UniqueHits;
	BertaComboGraphTargetingRules::StableUniqueActorHits(Hits, UniqueHits);

	Result.TargetData = FGameplayAbilityTargetDataHandle();
	Result.AddTargets(UniqueHits, TArray<AActor*>());
	return Result;
}
