#include "Targeting/BertaGSCTargetTypes.h"

#include "BertaGASCompanionExt.h"
#include "Debug/BertaDebugDraw.h"
#include "GameFramework/Actor.h"
#include "TargetingSystem/TargetingPreset.h"
#include "TargetingSystem/TargetingSubsystem.h"
#include "Types/TargetingSystemTypes.h"

void UBertaGSCTargetType_DebugProxy::GetTargets_Implementation(
	AActor* TargetingActor,
	FGameplayEventData EventData,
	TArray<FHitResult>& OutHitResults,
	TArray<AActor*>& OutActors) const
{
	if (!InnerTargetType)
	{
		UE_LOG(LogBertaGASCompanionExt, Warning, TEXT("[TargetDebugProxy] InnerTargetType is not configured on %s."), *GetClass()->GetPathName());
		return;
	}
	if (InnerTargetType->IsChildOf(StaticClass()))
	{
		UE_LOG(LogBertaGASCompanionExt, Error, TEXT("[TargetDebugProxy] Recursive debug proxy InnerTargetType %s is not allowed."), *InnerTargetType->GetPathName());
		return;
	}
	if (InnerTargetType->HasAnyClassFlags(CLASS_Abstract))
	{
		UE_LOG(LogBertaGASCompanionExt, Error, TEXT("[TargetDebugProxy] InnerTargetType %s is abstract."), *InnerTargetType->GetPathName());
		return;
	}

	const int32 InitialHitCount = OutHitResults.Num();
	const int32 InitialActorCount = OutActors.Num();
	const UGSCTargetType* InnerTargetTypeDefaultObject = InnerTargetType->GetDefaultObject<UGSCTargetType>();
	InnerTargetTypeDefaultObject->GetTargets(TargetingActor, EventData, OutHitResults, OutActors);

	if (bLogSummary)
	{
		UE_LOG(
			LogBertaGASCompanionExt,
			Log,
			TEXT("[TargetDebugProxy] TargetingActor=%s Inner=%s ProducedHits=%d ProducedActors=%d"),
			*GetPathNameSafe(TargetingActor),
			*InnerTargetType->GetPathName(),
			OutHitResults.Num() - InitialHitCount,
			OutActors.Num() - InitialActorCount);
	}

	if (!bDebugDraw || !TargetingActor)
	{
		return;
	}

	for (int32 Index = InitialHitCount; Index < OutHitResults.Num(); ++Index)
	{
		const FHitResult& HitResult = OutHitResults[Index];
		const FVector Point = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.Location;
		UBertaDebugDraw::DrawPoint(TargetingActor, true, Point, FLinearColor::Yellow, DebugDrawDuration, DebugPointSize);
		UBertaDebugDraw::DrawString(
			TargetingActor,
			true,
			Point,
			FString::Printf(TEXT("Hit[%d] %s"), Index - InitialHitCount, *GetNameSafe(HitResult.GetActor())),
			FLinearColor::Yellow,
			DebugDrawDuration);
	}
	for (int32 Index = InitialActorCount; Index < OutActors.Num(); ++Index)
	{
		const AActor* TargetActor = OutActors[Index];
		if (!IsValid(TargetActor))
		{
			continue;
		}
		UBertaDebugDraw::DrawPoint(TargetingActor, true, TargetActor->GetActorLocation(), FLinearColor::Green, DebugDrawDuration, DebugPointSize);
		UBertaDebugDraw::DrawString(
			TargetingActor,
			true,
			TargetActor->GetActorLocation(),
			FString::Printf(TEXT("Actor[%d] %s"), Index - InitialActorCount, *TargetActor->GetName()),
			FLinearColor::Green,
			DebugDrawDuration);
	}
}

void UBertaGSCTargetType_TargetingPreset::GetTargets_Implementation(
	AActor* TargetingActor,
	FGameplayEventData EventData,
	TArray<FHitResult>& OutHitResults,
	TArray<AActor*>& OutActors) const
{
	if (!IsValid(TargetingActor) || !TargetingPreset)
	{
		UE_LOG(LogBertaGASCompanionExt, Warning, TEXT("[TargetingPreset] Valid TargetingActor and TargetingPreset are required."));
		return;
	}

	UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::GetTargetingSubsystem(TargetingActor);
	if (!TargetingSubsystem)
	{
		UE_LOG(LogBertaGASCompanionExt, Warning, TEXT("[TargetingPreset] No TargetingSubsystem is available for %s."), *TargetingActor->GetPathName());
		return;
	}

	FTargetingSourceContext SourceContext;
	SourceContext.SourceActor = TargetingActor;
	SourceContext.InstigatorActor = const_cast<AActor*>(EventData.Instigator.Get());
	SourceContext.SourceLocation = TargetingActor->GetActorLocation();
	SourceContext.SourceObject = const_cast<UObject*>(EventData.OptionalObject.Get());

	FTargetingRequestHandle RequestHandle = UTargetingSubsystem::MakeTargetRequestHandle(TargetingPreset, SourceContext);
	if (!RequestHandle.IsValid())
	{
		UE_LOG(LogBertaGASCompanionExt, Warning, TEXT("[TargetingPreset] Failed to create an immediate request for %s."), *TargetingPreset->GetPathName());
		return;
	}

	TargetingSubsystem->ExecuteTargetingRequestWithHandle(RequestHandle);
	TArray<FHitResult> NativeHitResults;
	TArray<AActor*> NativeActors;
	TargetingSubsystem->GetTargetingResults(RequestHandle, NativeHitResults);
	TargetingSubsystem->GetTargetingResultsActors(RequestHandle, NativeActors);
	UTargetingSubsystem::ReleaseTargetRequestHandle(RequestHandle);

	TSet<const AActor*> ActorsRepresentedByHits;
	for (const FHitResult& HitResult : NativeHitResults)
	{
		if (const AActor* HitActor = HitResult.GetActor())
		{
			ActorsRepresentedByHits.Add(HitActor);
		}
	}
	OutHitResults.Append(NativeHitResults);
	for (AActor* NativeActor : NativeActors)
	{
		if (IsValid(NativeActor) && !ActorsRepresentedByHits.Contains(NativeActor))
		{
			OutActors.AddUnique(NativeActor);
		}
	}
}
