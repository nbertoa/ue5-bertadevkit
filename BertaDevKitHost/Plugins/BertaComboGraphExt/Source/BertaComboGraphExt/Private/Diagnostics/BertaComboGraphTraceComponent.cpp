#include "Diagnostics/BertaComboGraphTraceComponent.h"

#include "BertaComboGraphExt.h"
#include "Diagnostics/BertaComboGraphTraceRules.h"
#include "Abilities/Tasks/ComboGraphAbilityTask_StartGraph.h"
#include "Animation/AnimMontage.h"
#include "ComboGraphDelegates.h"
#include "Engine/World.h"
#include "Graph/ComboGraph.h"
#include "Graph/ComboGraphEdge.h"
#include "Graph/ComboGraphNodeAnimBase.h"
#include "TimerManager.h"

namespace
{
	const TCHAR* TraceTypeName(const EBertaComboGraphTraceEventType Type)
	{
		if (const UEnum* Enum = StaticEnum<EBertaComboGraphTraceEventType>()) return *Enum->GetNameStringByValue(static_cast<int64>(Type));
		return TEXT("Unknown");
	}
}

UBertaComboGraphTraceComponent::UBertaComboGraphTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBertaComboGraphTraceComponent::StartTracing()
{
	if (bTracing) return;
	bTracing = true;
	TraceStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	StartedHandle = FComboGraphDelegates::OnComboGraphStarted.AddUObject(this, &ThisClass::HandleGraphStarted);
	EndedHandle = FComboGraphDelegates::OnComboGraphEnded.AddUObject(this, &ThisClass::HandleGraphEnded);
	BoundInputBuffer = GetOwner() ? GetOwner()->FindComponentByClass<UBertaComboGraphInputBufferComponent>() : nullptr;
	if (BoundInputBuffer) BoundInputBuffer->OnInputBufferDiagnostic.AddUniqueDynamic(this, &ThisClass::HandleInputBufferDiagnostic);
}

void UBertaComboGraphTraceComponent::StopTracing()
{
	if (!bTracing) return;
	FComboGraphDelegates::OnComboGraphStarted.Remove(StartedHandle);
	FComboGraphDelegates::OnComboGraphEnded.Remove(EndedHandle);
	if (BoundInputBuffer) BoundInputBuffer->OnInputBufferDiagnostic.RemoveDynamic(this, &ThisClass::HandleInputBufferDiagnostic);
	BoundInputBuffer = nullptr;
	for (FExecutionRecord& Record : Executions)
	{
		if (UComboGraphAbilityTask_StartGraph* Task = Record.Task.Get()) Task->EventReceived.RemoveDynamic(this, &ThisClass::HandleGameplayEvent);
	}
	Executions.Reset();
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(SamplingTimer);
	bTracing = false;
}

void UBertaComboGraphTraceComponent::ClearTrace()
{
	Events.Reset();
	TraceStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

TArray<FBertaComboGraphStateSnapshot> UBertaComboGraphTraceComponent::GetStateSnapshots() const
{
	TArray<FBertaComboGraphStateSnapshot> Result;
	for (const FExecutionRecord& Record : Executions)
	{
		const UComboGraphAbilityTask_StartGraph* Task = Record.Task.Get();
		if (!Task) continue;
		FBertaComboGraphStateSnapshot Snapshot;
		Snapshot.ActorPath = GetPathNameSafe(GetOwner());
		Snapshot.GraphPath = GetPathNameSafe(Record.Graph.Get());
		Snapshot.TaskPath = GetPathNameSafe(Task);
		Snapshot.CurrentNodePath = GetPathNameSafe(Task->GetCurrentNode());
		Snapshot.PreviousNodePath = GetPathNameSafe(Task->GetPreviousNode());
		Snapshot.QueuedNodePath = GetPathNameSafe(Task->GetQueuedNode());
		Snapshot.MontagePath = GetPathNameSafe(Task->GetCurrentMontageFromAbility());
		Snapshot.bComboWindowOpen = Task->IsComboWindowOpened();
		Snapshot.bTransitionAppearsQueued = Task->GetQueuedNode() != nullptr;
		Snapshot.RuntimeTimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		Snapshot.ActiveGraphCount = Executions.Num();
		Result.Add(MoveTemp(Snapshot));
	}
	Result.Sort([](const FBertaComboGraphStateSnapshot& A, const FBertaComboGraphStateSnapshot& B) { return A.TaskPath < B.TaskPath; });
	return Result;
}

FString UBertaComboGraphTraceComponent::DumpTraceToText() const
{
	TArray<FString> Lines;
	for (const FBertaComboGraphTraceEvent& Event : Events) Lines.Add(FormatTraceEvent(Event));
	return FString::Join(Lines, TEXT("\n"));
}

FString UBertaComboGraphTraceComponent::FormatTraceEvent(const FBertaComboGraphTraceEvent& Event)
{
	return FString::Printf(TEXT("[+%.3fs] %s [%s] Execution=%s Graph=%s Previous=%s Current=%s Queued=%s ResolvedEdge=%s Event=%s Targets=%d Hit=%s Magnitude=%.3f | %s"),
		Event.RelativeTimeSeconds, TraceTypeName(Event.Type), *StaticEnum<EBertaComboGraphObservationKind>()->GetNameStringByValue(static_cast<int64>(Event.Observation)),
		*Event.ExecutionId, *Event.GraphPath, *Event.PreviousNodePath, *Event.CurrentNodePath, *Event.QueuedNodePath, *Event.ResolvedEdgePath,
		*Event.GameplayEventTag.ToString(), Event.TargetDataCount, Event.bHasHitResult ? TEXT("Yes") : TEXT("No"), Event.EventMagnitude, *Event.Message);
}

FString UBertaComboGraphTraceComponent::FormatStateSnapshot(const FBertaComboGraphStateSnapshot& S)
{
	return FString::Printf(TEXT("Actor=%s Graph=%s Task=%s Current=%s Previous=%s Queued=%s Montage=%s Window=%s TransitionQueued=%s ActiveGraphs=%d Time=%.3f"),
		*S.ActorPath, *S.GraphPath, *S.TaskPath, *S.CurrentNodePath, *S.PreviousNodePath, *S.QueuedNodePath, *S.MontagePath,
		S.bComboWindowOpen ? TEXT("Open") : TEXT("Closed"), S.bTransitionAppearsQueued ? TEXT("Yes") : TEXT("No"), S.ActiveGraphCount, S.RuntimeTimestampSeconds);
}

void UBertaComboGraphTraceComponent::EndPlay(const EEndPlayReason::Type Reason) { StopTracing(); Super::EndPlay(Reason); }
void UBertaComboGraphTraceComponent::BeginDestroy() { StopTracing(); Super::BeginDestroy(); }

bool UBertaComboGraphTraceComponent::IsTaskForOwner(const UComboGraphAbilityTask_StartGraph& Task) const
{
	return Task.GetAvatarActorFromActorInfo() == GetOwner();
}

FString UBertaComboGraphTraceComponent::ExecutionId(const UComboGraphAbilityTask_StartGraph* Task, const UComboGraph* Graph)
{
	return FString::Printf(TEXT("%s|%s"), *GetPathNameSafe(Task), *GetPathNameSafe(Graph));
}

FBertaComboGraphTraceEvent UBertaComboGraphTraceComponent::MakeEvent(const FExecutionRecord& Record, const EBertaComboGraphTraceEventType Type, const EBertaComboGraphObservationKind Observation) const
{
	FBertaComboGraphTraceEvent Event;
	Event.Type = Type;
	Event.Observation = Observation;
	Event.bInferred = Observation == EBertaComboGraphObservationKind::Inferred;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Event.RelativeTimeSeconds = FMath::Max(0.0f, Now - TraceStartTime);
	Event.ExecutionId = ExecutionId(Record.Task.Get(), Record.Graph.Get());
	Event.GraphPath = GetPathNameSafe(Record.Graph.Get());
	Event.PreviousNodePath = GetPathNameSafe(Record.PreviousNode.Get());
	Event.CurrentNodePath = GetPathNameSafe(Record.CurrentNode.Get());
	Event.QueuedNodePath = GetPathNameSafe(Record.QueuedNode.Get());
	Event.MontagePath = GetPathNameSafe(Record.Montage.Get());
	return Event;
}

void UBertaComboGraphTraceComponent::AddEvent(FBertaComboGraphTraceEvent&& Event)
{
	if (Event.bInferred && !bIncludeInferredEvents) return;
	if (bLogEvents) UE_LOG(LogBertaComboGraphExt, Log, TEXT("%s"), *FormatTraceEvent(Event));
	BertaComboGraphTraceRules::AppendBounded(Events, MoveTemp(Event), MaximumEventCount);
}

void UBertaComboGraphTraceComponent::HandleGraphStarted(const UComboGraphAbilityTask_StartGraph& Task, const UComboGraph& Graph)
{
	if (!bTracing || !IsTaskForOwner(Task)) return;
	FExecutionRecord& Record = Executions.AddDefaulted_GetRef();
	Record.Task = const_cast<UComboGraphAbilityTask_StartGraph*>(&Task);
	Record.Graph = const_cast<UComboGraph*>(&Graph);
	Record.CurrentNode = Task.GetCurrentNode();
	Record.PreviousNode = Task.GetPreviousNode();
	Record.QueuedNode = Task.GetQueuedNode();
	Record.Montage = Task.GetCurrentMontageFromAbility();
	Record.bWindowOpen = Task.IsComboWindowOpened();
	RebindGameplayEventDelegate();
	AddEvent(MakeEvent(Record, EBertaComboGraphTraceEventType::GraphStarted, EBertaComboGraphObservationKind::Direct));
	UpdateSamplingTimer();
}

void UBertaComboGraphTraceComponent::HandleGraphEnded(const UComboGraphAbilityTask_StartGraph& Task, const UComboGraph& Graph)
{
	for (int32 Index = Executions.Num() - 1; Index >= 0; --Index)
	{
		if (Executions[Index].Task.Get() != &Task) continue;
		SampleExecution(Executions[Index]);
		if (Executions[Index].CurrentNode.IsValid()) AddEvent(MakeEvent(Executions[Index], EBertaComboGraphTraceEventType::NodeExited, EBertaComboGraphObservationKind::Inferred));
		AddEvent(MakeEvent(Executions[Index], EBertaComboGraphTraceEventType::GraphEnded, EBertaComboGraphObservationKind::Direct));
		const_cast<UComboGraphAbilityTask_StartGraph&>(Task).EventReceived.RemoveDynamic(this, &ThisClass::HandleGameplayEvent);
		Executions.RemoveAt(Index);
	}
	RebindGameplayEventDelegate();
	UpdateSamplingTimer();
}

void UBertaComboGraphTraceComponent::HandleGameplayEvent(const FGameplayTag EventTag, const FGameplayEventData EventData)
{
	// The public delegate does not include its sender. Bind it only when attribution is unambiguous.
	if (Executions.Num() == 1)
	{
		FBertaComboGraphTraceEvent Event = MakeEvent(Executions[0], EBertaComboGraphTraceEventType::GameplayEvent, EBertaComboGraphObservationKind::Direct);
		Event.GameplayEventTag = EventTag;
		Event.InstigatorPath = GetPathNameSafe(EventData.Instigator.Get());
		Event.TargetPath = GetPathNameSafe(EventData.Target.Get());
		Event.TargetDataCount = EventData.TargetData.Num();
		Event.bHasHitResult = EventData.ContextHandle.GetHitResult() != nullptr;
		Event.EventMagnitude = EventData.EventMagnitude;
		AddEvent(MoveTemp(Event));
	}
}

void UBertaComboGraphTraceComponent::HandleInputBufferDiagnostic(const FBertaComboGraphInputBufferEvent& Diagnostic)
{
	FBertaComboGraphTraceEvent Event;
	Event.RelativeTimeSeconds = FMath::Max(0.0f, Diagnostic.TimestampSeconds - TraceStartTime);
	Event.Observation = EBertaComboGraphObservationKind::Direct;
	Event.ExecutionId = Diagnostic.ExecutionId;
	Event.CurrentNodePath = Diagnostic.NodePath;
	Event.Message = FString::Printf(TEXT("Input=%s | %s"), *Diagnostic.InputActionPath, *Diagnostic.Reason);
	switch (Diagnostic.Type)
	{
	case EBertaComboGraphInputBufferEventType::Buffered: Event.Type = EBertaComboGraphTraceEventType::InputBuffered; break;
	case EBertaComboGraphInputBufferEventType::Consumed: Event.Type = EBertaComboGraphTraceEventType::InputBufferConsumed; break;
	case EBertaComboGraphInputBufferEventType::Expired: Event.Type = EBertaComboGraphTraceEventType::InputBufferExpired; break;
	case EBertaComboGraphInputBufferEventType::Rejected: Event.Type = EBertaComboGraphTraceEventType::InputBufferRejected; break;
	default: Event.Type = EBertaComboGraphTraceEventType::InputBufferCleared; break;
	}
	AddEvent(MoveTemp(Event));
}

void UBertaComboGraphTraceComponent::RebindGameplayEventDelegate()
{
	for (FExecutionRecord& Record : Executions)
	{
		if (UComboGraphAbilityTask_StartGraph* Task = Record.Task.Get()) Task->EventReceived.RemoveDynamic(this, &ThisClass::HandleGameplayEvent);
	}
	if (Executions.Num() == 1)
	{
		if (UComboGraphAbilityTask_StartGraph* Task = Executions[0].Task.Get()) Task->EventReceived.AddUniqueDynamic(this, &ThisClass::HandleGameplayEvent);
	}
}

void UBertaComboGraphTraceComponent::SampleExecutions()
{
	bool bRemovedInvalidExecution = false;
	for (int32 Index = Executions.Num() - 1; Index >= 0; --Index)
	{
		if (!Executions[Index].Task.IsValid())
		{
			Executions.RemoveAt(Index);
			bRemovedInvalidExecution = true;
		}
		else SampleExecution(Executions[Index]);
	}
	if (bRemovedInvalidExecution) RebindGameplayEventDelegate();
	UpdateSamplingTimer();
}

void UBertaComboGraphTraceComponent::SampleExecution(FExecutionRecord& R)
{
	UComboGraphAbilityTask_StartGraph* Task = R.Task.Get();
	if (!Task) return;
	UComboGraphNodeAnimBase* NewCurrent = Task->GetCurrentNode();
	if (NewCurrent != R.CurrentNode.Get())
	{
		UComboGraphNodeAnimBase* OldCurrent = R.CurrentNode.Get();
		if (R.CurrentNode.IsValid()) AddEvent(MakeEvent(R, EBertaComboGraphTraceEventType::NodeExited, EBertaComboGraphObservationKind::Observed));
		R.PreviousNode = Task->GetPreviousNode();
		R.CurrentNode = NewCurrent;
		AddEvent(MakeEvent(R, EBertaComboGraphTraceEventType::NodeEntered, EBertaComboGraphObservationKind::Observed));
		if (OldCurrent && NewCurrent)
		{
			const UComboGraphEdge* ResolvedEdge = OldCurrent->GetEdge(NewCurrent);
			FBertaComboGraphTraceEvent Transition = MakeEvent(R, EBertaComboGraphTraceEventType::TransitionObserved,
				ResolvedEdge ? EBertaComboGraphObservationKind::ResolvedFromTopology : EBertaComboGraphObservationKind::Observed);
			Transition.ResolvedEdgePath = GetPathNameSafe(ResolvedEdge);
			Transition.Message = ResolvedEdge
				? TEXT("Connection asset resolved from previous/current topology; the responsible raw input is not proven.")
				: TEXT("Current/previous node change observed; the responsible edge or raw input is not proven.");
			AddEvent(MoveTemp(Transition));
		}
	}
	if (Task->GetQueuedNode() != R.QueuedNode.Get()) { R.QueuedNode = Task->GetQueuedNode(); AddEvent(MakeEvent(R, EBertaComboGraphTraceEventType::QueuedNodeChanged, EBertaComboGraphObservationKind::Observed)); }
	if (Task->GetCurrentMontageFromAbility() != R.Montage.Get()) { R.Montage = Task->GetCurrentMontageFromAbility(); AddEvent(MakeEvent(R, EBertaComboGraphTraceEventType::MontageChanged, EBertaComboGraphObservationKind::Observed)); }
	if (Task->IsComboWindowOpened() != R.bWindowOpen)
	{
		R.bWindowOpen = Task->IsComboWindowOpened();
		AddEvent(MakeEvent(R, R.bWindowOpen ? EBertaComboGraphTraceEventType::ComboWindowOpened : EBertaComboGraphTraceEventType::ComboWindowClosed, EBertaComboGraphObservationKind::Observed));
	}
}

void UBertaComboGraphTraceComponent::UpdateSamplingTimer()
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (bTracing && Executions.Num() > 0 && !World->GetTimerManager().IsTimerActive(SamplingTimer))
	{
		World->GetTimerManager().SetTimer(SamplingTimer, this, &ThisClass::SampleExecutions, FMath::Clamp(ActiveSamplingIntervalSeconds, 0.01f, 1.0f), true);
	}
	else if (Executions.IsEmpty())
	{
		World->GetTimerManager().ClearTimer(SamplingTimer);
	}
}
