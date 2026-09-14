#include "Components/BertaGSCTraceComponent.h"

#include "BertaGASCompanionExt.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/GSCCoreComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"

namespace BertaGSCTracePrivate
{
	FString AbilityPath(const UGameplayAbility* Ability)
	{
		return Ability ? Ability->GetClass()->GetPathName() : TEXT("None");
	}

	FString SortedTags(const FGameplayTagContainer& Tags)
	{
		TArray<FString> Names;
		for (const FGameplayTag& Tag : Tags.GetGameplayTagArray())
		{
			Names.Add(Tag.ToString());
		}
		Names.Sort();
		return FString::Join(Names, TEXT(","));
	}

	const TCHAR* TypeName(const EBertaGSCTraceEventType Type)
	{
		switch (Type)
		{
		case EBertaGSCTraceEventType::AbilityActivated: return TEXT("AbilityActivated");
		case EBertaGSCTraceEventType::AbilityEnded: return TEXT("AbilityEnded");
		case EBertaGSCTraceEventType::AbilityFailed: return TEXT("AbilityFailed");
		case EBertaGSCTraceEventType::AbilityCommitted: return TEXT("AbilityCommitted");
		case EBertaGSCTraceEventType::CooldownStarted: return TEXT("CooldownStarted");
		case EBertaGSCTraceEventType::CooldownEnded: return TEXT("CooldownEnded");
		case EBertaGSCTraceEventType::GameplayEffectAdded: return TEXT("GameplayEffectAdded");
		case EBertaGSCTraceEventType::GameplayEffectRemoved: return TEXT("GameplayEffectRemoved");
		case EBertaGSCTraceEventType::GameplayEffectStackChanged: return TEXT("GameplayEffectStackChanged");
		case EBertaGSCTraceEventType::GameplayEffectTimeChanged: return TEXT("GameplayEffectTimeChanged");
		case EBertaGSCTraceEventType::GameplayTagChanged: return TEXT("GameplayTagChanged");
		case EBertaGSCTraceEventType::AttributeChanged: return TEXT("AttributeChanged");
		default: return TEXT("Unknown");
		}
	}
}

UBertaGSCTraceComponent::UBertaGSCTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UBertaGSCTraceComponent::StartTracing()
{
	if (BoundCoreComponent)
	{
		return true;
	}

	AActor* Owner = GetOwner();
	UGSCCoreComponent* CoreComponent = Owner ? Owner->FindComponentByClass<UGSCCoreComponent>() : nullptr;
	if (!CoreComponent)
	{
		return false;
	}

	CoreComponent->OnAbilityActivated.AddUniqueDynamic(this, &ThisClass::HandleAbilityActivated);
	CoreComponent->OnAbilityEnded.AddUniqueDynamic(this, &ThisClass::HandleAbilityEnded);
	CoreComponent->OnAbilityFailed.AddUniqueDynamic(this, &ThisClass::HandleAbilityFailed);
	CoreComponent->OnAbilityCommit.AddUniqueDynamic(this, &ThisClass::HandleAbilityCommitted);
	CoreComponent->OnCooldownStart.AddUniqueDynamic(this, &ThisClass::HandleCooldownStarted);
	CoreComponent->OnCooldownEnd.AddUniqueDynamic(this, &ThisClass::HandleCooldownEnded);
	CoreComponent->OnGameplayEffectAdded.AddUniqueDynamic(this, &ThisClass::HandleGameplayEffectAdded);
	CoreComponent->OnGameplayEffectRemoved.AddUniqueDynamic(this, &ThisClass::HandleGameplayEffectRemoved);
	CoreComponent->OnGameplayEffectStackChange.AddUniqueDynamic(this, &ThisClass::HandleGameplayEffectStackChanged);
	CoreComponent->OnGameplayEffectTimeChange.AddUniqueDynamic(this, &ThisClass::HandleGameplayEffectTimeChanged);
	CoreComponent->OnGameplayTagChange.AddUniqueDynamic(this, &ThisClass::HandleGameplayTagChanged);
	CoreComponent->OnAttributeChange.AddUniqueDynamic(this, &ThisClass::HandleAttributeChanged);

	BoundCoreComponent = CoreComponent;
	LastObservedAttributeValues.Reset();
	TraceStartWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return true;
}

void UBertaGSCTraceComponent::StopTracing()
{
	if (!BoundCoreComponent)
	{
		return;
	}

	BoundCoreComponent->OnAbilityActivated.RemoveDynamic(this, &ThisClass::HandleAbilityActivated);
	BoundCoreComponent->OnAbilityEnded.RemoveDynamic(this, &ThisClass::HandleAbilityEnded);
	BoundCoreComponent->OnAbilityFailed.RemoveDynamic(this, &ThisClass::HandleAbilityFailed);
	BoundCoreComponent->OnAbilityCommit.RemoveDynamic(this, &ThisClass::HandleAbilityCommitted);
	BoundCoreComponent->OnCooldownStart.RemoveDynamic(this, &ThisClass::HandleCooldownStarted);
	BoundCoreComponent->OnCooldownEnd.RemoveDynamic(this, &ThisClass::HandleCooldownEnded);
	BoundCoreComponent->OnGameplayEffectAdded.RemoveDynamic(this, &ThisClass::HandleGameplayEffectAdded);
	BoundCoreComponent->OnGameplayEffectRemoved.RemoveDynamic(this, &ThisClass::HandleGameplayEffectRemoved);
	BoundCoreComponent->OnGameplayEffectStackChange.RemoveDynamic(this, &ThisClass::HandleGameplayEffectStackChanged);
	BoundCoreComponent->OnGameplayEffectTimeChange.RemoveDynamic(this, &ThisClass::HandleGameplayEffectTimeChanged);
	BoundCoreComponent->OnGameplayTagChange.RemoveDynamic(this, &ThisClass::HandleGameplayTagChanged);
	BoundCoreComponent->OnAttributeChange.RemoveDynamic(this, &ThisClass::HandleAttributeChanged);
	BoundCoreComponent = nullptr;
	GameplayEffectPaths.Reset();
	LastObservedAttributeValues.Reset();
}

void UBertaGSCTraceComponent::ClearTrace()
{
	Events.Reset();
	LastObservedAttributeValues.Reset();
	TraceStartWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

TArray<FBertaGSCTraceEvent> UBertaGSCTraceComponent::GetTraceEvents() const
{
	return Events;
}

FString UBertaGSCTraceComponent::DumpTraceToText() const
{
	TArray<FString> Lines;
	Lines.Reserve(Events.Num());
	for (const FBertaGSCTraceEvent& Event : Events)
	{
		Lines.Add(FormatTraceEvent(Event));
	}
	return FString::Join(Lines, TEXT("\n"));
}

bool UBertaGSCTraceComponent::IsTracing() const
{
	return BoundCoreComponent != nullptr;
}

FString UBertaGSCTraceComponent::FormatTraceEvent(const FBertaGSCTraceEvent& Event)
{
	const FString OldValueText = Event.bOldValueKnown
		? FString::Printf(TEXT("%.3f"), Event.OldValue)
		: TEXT("unknown");
	return FString::Printf(
		TEXT("[+%.3fs @ %.3fs] %s | Actor=%s | Ability=%s | Effect=%s | Tag=%s Present=%s | Tags=[%s] | FailureTags=[%s] | Attribute=%s | Old=%s New=%.3f Delta=%.3f | Remaining=%.3fs Duration=%.3fs EffectStart=%.3fs | %s"),
		Event.RelativeTimeSeconds,
		Event.WorldTimeSeconds,
		BertaGSCTracePrivate::TypeName(Event.Type),
		*Event.ActorPath,
		Event.AbilityClassPath.IsEmpty() ? TEXT("None") : *Event.AbilityClassPath,
		Event.GameplayEffectClassPath.IsEmpty() ? TEXT("None") : *Event.GameplayEffectClassPath,
		*Event.GameplayTag.ToString(),
		Event.bGameplayTagPresent ? TEXT("Yes") : TEXT("No"),
		*BertaGSCTracePrivate::SortedTags(Event.GameplayTags),
		*BertaGSCTracePrivate::SortedTags(Event.FailureTags),
		Event.AttributeName.IsEmpty() ? TEXT("None") : *Event.AttributeName,
		*OldValueText,
		Event.NewValue,
		Event.DeltaValue,
		Event.TimeRemainingSeconds,
		Event.DurationSeconds,
		Event.GameplayEffectStartTimeSeconds,
		*Event.Message);
}

void UBertaGSCTraceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopTracing();
	Super::EndPlay(EndPlayReason);
}

void UBertaGSCTraceComponent::BeginDestroy()
{
	StopTracing();
	Super::BeginDestroy();
}

FBertaGSCTraceEvent UBertaGSCTraceComponent::MakeEvent(const EBertaGSCTraceEventType Type) const
{
	FBertaGSCTraceEvent Event;
	Event.Type = Type;
	Event.WorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Event.RelativeTimeSeconds = FMath::Max(0.0f, Event.WorldTimeSeconds - TraceStartWorldTimeSeconds);
	Event.ActorPath = GetPathNameSafe(GetOwner());
	return Event;
}

void UBertaGSCTraceComponent::AddEvent(FBertaGSCTraceEvent&& Event)
{
	const int32 ClampedMaximum = FMath::Clamp(MaximumEventCount, 1, 10000);
	const int32 ExcessCount = Events.Num() - ClampedMaximum + 1;
	if (ExcessCount > 0)
	{
		Events.RemoveAt(0, ExcessCount, EAllowShrinking::No);
	}

	if (Event.Message.IsEmpty())
	{
		Event.Message = BertaGSCTracePrivate::TypeName(Event.Type);
	}
	if (bLogEvents)
	{
		UE_LOG(LogBertaGASCompanionExt, Log, TEXT("%s"), *FormatTraceEvent(Event));
	}
	Events.Add(MoveTemp(Event));
}

FString UBertaGSCTraceComponent::ResolveGameplayEffectPath(const FActiveGameplayEffectHandle ActiveHandle) const
{
	if (const FString* CachedPath = GameplayEffectPaths.Find(ActiveHandle))
	{
		return *CachedPath;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ActiveHandle.GetOwningAbilitySystemComponent();
	const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent
		? AbilitySystemComponent->GetActiveGameplayEffect(ActiveHandle)
		: nullptr;
	return ActiveEffect && ActiveEffect->Spec.Def
		? ActiveEffect->Spec.Def->GetClass()->GetPathName()
		: TEXT("None");
}

void UBertaGSCTraceComponent::HandleAbilityActivated(const UGameplayAbility* Ability)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::AbilityActivated);
	Event.AbilityClassPath = BertaGSCTracePrivate::AbilityPath(Ability);
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleAbilityEnded(const UGameplayAbility* Ability)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::AbilityEnded);
	Event.AbilityClassPath = BertaGSCTracePrivate::AbilityPath(Ability);
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& ReasonTags)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::AbilityFailed);
	Event.AbilityClassPath = BertaGSCTracePrivate::AbilityPath(Ability);
	Event.FailureTags = ReasonTags;
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleAbilityCommitted(UGameplayAbility* Ability)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::AbilityCommitted);
	Event.AbilityClassPath = BertaGSCTracePrivate::AbilityPath(Ability);
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleCooldownStarted(UGameplayAbility* Ability, const FGameplayTagContainer CooldownTags, const float TimeRemaining, const float Duration)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::CooldownStarted);
	Event.AbilityClassPath = BertaGSCTracePrivate::AbilityPath(Ability);
	Event.GameplayTags = CooldownTags;
	Event.TimeRemainingSeconds = TimeRemaining;
	Event.DurationSeconds = Duration;
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleCooldownEnded(UGameplayAbility* Ability, const FGameplayTag CooldownTag, const float Duration)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::CooldownEnded);
	Event.AbilityClassPath = BertaGSCTracePrivate::AbilityPath(Ability);
	Event.GameplayTag = CooldownTag;
	Event.DurationSeconds = Duration;
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleGameplayEffectAdded(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, const FActiveGameplayEffectHandle ActiveHandle)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::GameplayEffectAdded);
	Event.GameplayEffectClassPath = ResolveGameplayEffectPath(ActiveHandle);
	Event.GameplayTags = AssetTags;
	Event.GameplayTags.AppendTags(GrantedTags);
	GameplayEffectPaths.Add(ActiveHandle, Event.GameplayEffectClassPath);
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleGameplayEffectRemoved(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, const FActiveGameplayEffectHandle ActiveHandle)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::GameplayEffectRemoved);
	Event.GameplayEffectClassPath = ResolveGameplayEffectPath(ActiveHandle);
	Event.GameplayTags = AssetTags;
	Event.GameplayTags.AppendTags(GrantedTags);
	GameplayEffectPaths.Remove(ActiveHandle);
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleGameplayEffectStackChanged(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, const FActiveGameplayEffectHandle ActiveHandle, const int32 NewStackCount, const int32 OldStackCount)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::GameplayEffectStackChanged);
	Event.GameplayEffectClassPath = ResolveGameplayEffectPath(ActiveHandle);
	Event.GameplayTags = AssetTags;
	Event.GameplayTags.AppendTags(GrantedTags);
	Event.bOldValueKnown = true;
	Event.OldValue = OldStackCount;
	Event.NewValue = NewStackCount;
	Event.DeltaValue = NewStackCount - OldStackCount;
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleGameplayEffectTimeChanged(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, const FActiveGameplayEffectHandle ActiveHandle, const float NewStartTime, const float NewDuration)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::GameplayEffectTimeChanged);
	Event.GameplayEffectClassPath = ResolveGameplayEffectPath(ActiveHandle);
	Event.GameplayTags = AssetTags;
	Event.GameplayTags.AppendTags(GrantedTags);
	Event.GameplayEffectStartTimeSeconds = NewStartTime;
	Event.DurationSeconds = NewDuration;
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleGameplayTagChanged(const FGameplayTag ChangedTag, const int32 NewTagCount)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::GameplayTagChanged);
	Event.GameplayTag = ChangedTag;
	Event.NewValue = NewTagCount;
	Event.bGameplayTagPresent = NewTagCount > 0;
	AddEvent(MoveTemp(Event));
}

void UBertaGSCTraceComponent::HandleAttributeChanged(const FGameplayAttribute Attribute, const float DeltaValue, const FGameplayTagContainer EventTags)
{
	FBertaGSCTraceEvent Event = MakeEvent(EBertaGSCTraceEventType::AttributeChanged);
	Event.AttributeName = Attribute.GetName();
	Event.DeltaValue = DeltaValue;
	Event.GameplayTags = EventTags;
	Event.NewValue = BoundCoreComponent ? BoundCoreComponent->GetCurrentAttributeValue(Attribute) : 0.0f;
	if (const float* PreviousValue = LastObservedAttributeValues.Find(Attribute))
	{
		Event.bOldValueKnown = true;
		Event.OldValue = *PreviousValue;
	}
	LastObservedAttributeValues.Add(Attribute, Event.NewValue);
	AddEvent(MoveTemp(Event));
}
