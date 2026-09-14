#include "Diagnostics/BertaGSCEffectiveLoadout.h"

#include "Diagnostics/BertaGSCEffectiveLoadoutInternal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"

namespace
{
	template <typename TEntry, typename TPathGetter>
	void SortByPath(TArray<TEntry>& Entries, TPathGetter&& GetPath)
	{
		Entries.Sort([&GetPath](const TEntry& Left, const TEntry& Right)
		{
			return GetPath(Left).Compare(GetPath(Right), ESearchCase::CaseSensitive) < 0;
		});
	}

	FString OptionalPath(const UObject* Object)
	{
		return Object ? Object->GetPathName() : FString();
	}
}

namespace BertaGSCEffectiveLoadout
{
	void SortSnapshot(FBertaGSCEffectiveLoadoutSnapshot& Snapshot)
	{
		SortByPath(Snapshot.Abilities, [](const FBertaGSCLoadoutAbility& Entry)
		{
			return Entry.Readiness.Inspection.AbilityClassPath + TEXT("|") + Entry.Readiness.AbilitySpecHandle.ToString();
		});
		SortByPath(Snapshot.ActiveGameplayEffects, [](const FBertaGSCLoadoutGameplayEffect& Entry)
		{
			return Entry.EffectClassPath + TEXT("|") + Entry.ActiveHandle;
		});
		SortByPath(Snapshot.AttributeSets, [](const FBertaGSCLoadoutAttributeSet& Entry)
		{
			return Entry.AttributeSetClassPath + TEXT("|") + Entry.ObjectPath;
		});
		Snapshot.OwnedGameplayTags.Sort([](const FBertaGSCLoadoutGameplayTag& Left, const FBertaGSCLoadoutGameplayTag& Right)
		{
			return Left.Tag.ToString() < Right.Tag.ToString();
		});
	}
}

bool UBertaGSCEffectiveLoadoutLibrary::BuildEffectiveLoadoutSnapshot(
	AActor* Actor,
	FBertaGSCEffectiveLoadoutSnapshot& OutSnapshot)
{
	OutSnapshot = FBertaGSCEffectiveLoadoutSnapshot();
	OutSnapshot.ActorPath = IsValid(Actor) ? Actor->GetPathName() : TEXT("None");
	if (!IsInGameThread() || !IsValid(Actor))
	{
		OutSnapshot.Summary = FormatEffectiveLoadoutSnapshot(OutSnapshot);
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!AbilitySystemComponent)
	{
		OutSnapshot.Summary = FormatEffectiveLoadoutSnapshot(OutSnapshot);
		return false;
	}

	OutSnapshot.bAbilitySystemComponentFound = true;
	OutSnapshot.AbilitySystemComponentPath = AbilitySystemComponent->GetPathName();

	FBertaGSCAbilityReadinessReport Readiness;
	UBertaGSCAbilityReadinessLibrary::BuildAbilityReadinessReport(Actor, Readiness);
	OutSnapshot.Abilities.Reserve(Readiness.Entries.Num());
	for (const FBertaGSCAbilityReadinessEntry& ReadinessEntry : Readiness.Entries)
	{
		FBertaGSCLoadoutAbility& Entry = OutSnapshot.Abilities.AddDefaulted_GetRef();
		Entry.Readiness = ReadinessEntry;
		const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(
			ReadinessEntry.AbilitySpecHandle);
		if (!Spec)
		{
			continue;
		}

		TArray<FString> Evidence;
		if (UObject* SourceObject = Spec->SourceObject.Get())
		{
			Entry.SourceObjectPath = SourceObject->GetPathName();
			Evidence.Add(FString::Printf(TEXT("Spec SourceObject=%s"), *Entry.SourceObjectPath));
		}

		const FActiveGameplayEffectHandle GrantingHandle =
			AbilitySystemComponent->FindActiveGameplayEffectHandle(Spec->Handle);
		if (const FActiveGameplayEffect* GrantingEffect =
			AbilitySystemComponent->GetActiveGameplayEffect(GrantingHandle))
		{
			if (GrantingEffect->Spec.Def)
			{
				Entry.GrantingGameplayEffectClassPath = GrantingEffect->Spec.Def->GetClass()->GetPathName();
				Evidence.Add(FString::Printf(
					TEXT("ASC links spec to active GameplayEffect=%s"),
					*Entry.GrantingGameplayEffectClassPath));
			}
		}

		if (!Evidence.IsEmpty())
		{
			Entry.ProvenanceConfidence = EBertaGSCProvenanceConfidence::Proven;
			Entry.ProvenanceEvidence = FString::Join(Evidence, TEXT("; "));
		}
	}

	const TArray<FActiveGameplayEffectHandle> EffectHandles =
		AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery());
	OutSnapshot.ActiveGameplayEffects.Reserve(EffectHandles.Num());
	const UWorld* World = AbilitySystemComponent->GetWorld();
	for (const FActiveGameplayEffectHandle Handle : EffectHandles)
	{
		const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent->GetActiveGameplayEffect(Handle);
		if (!ActiveEffect || !ActiveEffect->Spec.Def)
		{
			continue;
		}

		FBertaGSCLoadoutGameplayEffect& Entry = OutSnapshot.ActiveGameplayEffects.AddDefaulted_GetRef();
		Entry.EffectClass = ActiveEffect->Spec.Def->GetClass();
		Entry.EffectClassPath = ActiveEffect->Spec.Def->GetClass()->GetPathName();
		Entry.ActiveHandle = Handle.ToString();
		Entry.StackCount = ActiveEffect->Spec.GetStackCount();
		Entry.DurationSeconds = ActiveEffect->GetDuration();
		Entry.bInfiniteDuration = Entry.DurationSeconds < 0.0f;
		Entry.TimeRemainingSeconds = World
			? ActiveEffect->GetTimeRemaining(World->GetTimeSeconds())
			: 0.0f;

		const FGameplayEffectContextHandle& Context = ActiveEffect->Spec.GetContext();
		Entry.InstigatorPath = OptionalPath(Context.GetInstigator());
		Entry.EffectCauserPath = OptionalPath(Context.GetEffectCauser());
		Entry.SourceObjectPath = OptionalPath(Context.GetSourceObject());
		TArray<FString> Evidence;
		if (!Entry.InstigatorPath.IsEmpty())
		{
			Evidence.Add(FString::Printf(TEXT("Context Instigator=%s"), *Entry.InstigatorPath));
		}
		if (!Entry.EffectCauserPath.IsEmpty())
		{
			Evidence.Add(FString::Printf(TEXT("Context EffectCauser=%s"), *Entry.EffectCauserPath));
		}
		if (!Entry.SourceObjectPath.IsEmpty())
		{
			Evidence.Add(FString::Printf(TEXT("Context SourceObject=%s"), *Entry.SourceObjectPath));
		}
		if (!Evidence.IsEmpty())
		{
			Entry.ProvenanceConfidence = EBertaGSCProvenanceConfidence::Proven;
			Entry.ProvenanceEvidence = FString::Join(Evidence, TEXT("; "));
		}
	}

	for (UAttributeSet* AttributeSet : AbilitySystemComponent->GetSpawnedAttributes())
	{
		if (!AttributeSet)
		{
			continue;
		}
		FBertaGSCLoadoutAttributeSet& Entry = OutSnapshot.AttributeSets.AddDefaulted_GetRef();
		Entry.AttributeSetClass = AttributeSet->GetClass();
		Entry.AttributeSetClassPath = AttributeSet->GetClass()->GetPathName();
		Entry.ObjectPath = AttributeSet->GetPathName();
		Entry.ProvenanceEvidence = TEXT("No public runtime grant-source record is exposed for spawned AttributeSets.");
	}

	for (const FGameplayTag& Tag : AbilitySystemComponent->GetOwnedGameplayTags().GetGameplayTagArray())
	{
		FBertaGSCLoadoutGameplayTag& Entry = OutSnapshot.OwnedGameplayTags.AddDefaulted_GetRef();
		Entry.Tag = Tag;
		Entry.Count = AbilitySystemComponent->GetTagCount(Tag);
		Entry.ProvenanceEvidence = TEXT("The public ASC API exposes only the effective count, not a loose/effect source breakdown.");
	}

	BertaGSCEffectiveLoadout::SortSnapshot(OutSnapshot);
	OutSnapshot.Summary = FormatEffectiveLoadoutSnapshot(OutSnapshot);
	return true;
}

FString UBertaGSCEffectiveLoadoutLibrary::FormatEffectiveLoadoutSnapshot(
	const FBertaGSCEffectiveLoadoutSnapshot& Snapshot)
{
	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Actor: %s"), *Snapshot.ActorPath));
	Lines.Add(FString::Printf(
		TEXT("ASC: %s"), Snapshot.bAbilitySystemComponentFound ? *Snapshot.AbilitySystemComponentPath : TEXT("Not found")));
	Lines.Add(TEXT("\nAbilities\n---------"));
	for (const FBertaGSCLoadoutAbility& Entry : Snapshot.Abilities)
	{
		Lines.Add(FString::Printf(
			TEXT("%s\n  Handle: %s | Level: %d | Active: %s | Input: %s\n  SourceObject: %s\n  GrantingEffect: %s\n  Provenance: %s%s%s"),
			*Entry.Readiness.Inspection.AbilityClassPath,
			*Entry.Readiness.AbilitySpecHandle.ToString(),
			Entry.Readiness.Inspection.GrantedLevel,
			Entry.Readiness.Inspection.bAbilityActive ? TEXT("Yes") : TEXT("No"),
			*GetPathNameSafe(Entry.Readiness.Inspection.BoundInputAction),
			Entry.SourceObjectPath.IsEmpty() ? TEXT("None") : *Entry.SourceObjectPath,
			Entry.GrantingGameplayEffectClassPath.IsEmpty() ? TEXT("None") : *Entry.GrantingGameplayEffectClassPath,
			*FormatProvenanceConfidence(Entry.ProvenanceConfidence),
			Entry.ProvenanceEvidence.IsEmpty() ? TEXT("") : TEXT(" ("),
			Entry.ProvenanceEvidence.IsEmpty() ? TEXT("") : *FString(Entry.ProvenanceEvidence + TEXT(")"))));
	}

	Lines.Add(TEXT("\nGameplay Effects\n----------------"));
	for (const FBertaGSCLoadoutGameplayEffect& Entry : Snapshot.ActiveGameplayEffects)
	{
		Lines.Add(FString::Printf(
			TEXT("%s\n  Handle: %s | Stack: %d | Remaining: %s | Duration: %s\n  Instigator: %s | EffectCauser: %s | SourceObject: %s\n  Provenance: %s%s%s"),
			*Entry.EffectClassPath,
			*Entry.ActiveHandle,
			Entry.StackCount,
			Entry.bInfiniteDuration ? TEXT("Infinite") : *FString::Printf(TEXT("%.3fs"), Entry.TimeRemainingSeconds),
			Entry.bInfiniteDuration ? TEXT("Infinite") : *FString::Printf(TEXT("%.3fs"), Entry.DurationSeconds),
			Entry.InstigatorPath.IsEmpty() ? TEXT("None") : *Entry.InstigatorPath,
			Entry.EffectCauserPath.IsEmpty() ? TEXT("None") : *Entry.EffectCauserPath,
			Entry.SourceObjectPath.IsEmpty() ? TEXT("None") : *Entry.SourceObjectPath,
			*FormatProvenanceConfidence(Entry.ProvenanceConfidence),
			Entry.ProvenanceEvidence.IsEmpty() ? TEXT("") : TEXT(" ("),
			Entry.ProvenanceEvidence.IsEmpty() ? TEXT("") : *FString(Entry.ProvenanceEvidence + TEXT(")"))));
	}

	Lines.Add(TEXT("\nAttribute Sets\n--------------"));
	for (const FBertaGSCLoadoutAttributeSet& Entry : Snapshot.AttributeSets)
	{
		Lines.Add(FString::Printf(
			TEXT("%s\n  Object: %s\n  Provenance: %s%s%s"),
			*Entry.AttributeSetClassPath,
			*Entry.ObjectPath,
			*FormatProvenanceConfidence(Entry.ProvenanceConfidence),
			Entry.ProvenanceEvidence.IsEmpty() ? TEXT("") : TEXT(" ("),
			Entry.ProvenanceEvidence.IsEmpty() ? TEXT("") : *FString(Entry.ProvenanceEvidence + TEXT(")"))));
	}

	Lines.Add(TEXT("\nGameplay Tags\n-------------"));
	for (const FBertaGSCLoadoutGameplayTag& Entry : Snapshot.OwnedGameplayTags)
	{
		Lines.Add(FString::Printf(
			TEXT("%s | Count: %d | Provenance: %s%s%s"),
			*Entry.Tag.ToString(),
			Entry.Count,
			*FormatProvenanceConfidence(Entry.ProvenanceConfidence),
			Entry.ProvenanceEvidence.IsEmpty() ? TEXT("") : TEXT(" ("),
			Entry.ProvenanceEvidence.IsEmpty() ? TEXT("") : *FString(Entry.ProvenanceEvidence + TEXT(")"))));
	}
	return FString::Join(Lines, TEXT("\n"));
}

FString UBertaGSCEffectiveLoadoutLibrary::FormatProvenanceConfidence(
	const EBertaGSCProvenanceConfidence Confidence)
{
	switch (Confidence)
	{
	case EBertaGSCProvenanceConfidence::Proven: return TEXT("Proven");
	case EBertaGSCProvenanceConfidence::Inferred: return TEXT("Inferred");
	case EBertaGSCProvenanceConfidence::Unknown: return TEXT("Unknown");
	default: return TEXT("Unknown");
	}
}
