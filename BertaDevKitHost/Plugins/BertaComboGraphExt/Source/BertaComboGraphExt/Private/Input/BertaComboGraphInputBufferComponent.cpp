#include "Input/BertaComboGraphInputBufferComponent.h"
#include "Input/BertaComboGraphInputBufferRules.h"

#include "Abilities/ComboGraphNativeTags.h"
#include "Abilities/Tasks/ComboGraphAbilityTask_StartGraph.h"
#include "ComboGraphDelegates.h"
#include "Components/ComboGraphGameplayTasksComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "Graph/ComboGraph.h"
#include "Graph/ComboGraphEdge.h"
#include "Graph/ComboGraphNodeAnimBase.h"
#include "TimerManager.h"

UBertaComboGraphInputBufferComponent::UBertaComboGraphInputBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBertaComboGraphInputBufferComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bEnabled) StartBuffering();
}

bool UBertaComboGraphInputBufferComponent::StartBuffering()
{
	if (bBuffering) return true;
	BoundInputComponent = GetOwner() ? GetOwner()->FindComponentByClass<UEnhancedInputComponent>() : nullptr;
	StartedHandle = FComboGraphDelegates::OnComboGraphStarted.AddUObject(this, &ThisClass::HandleGraphStarted);
	EndedHandle = FComboGraphDelegates::OnComboGraphEnded.AddUObject(this, &ThisClass::HandleGraphEnded);
	bBuffering = true;
	return true;
}

void UBertaComboGraphInputBufferComponent::StopBuffering()
{
	if (!bBuffering) return;
	FComboGraphDelegates::OnComboGraphStarted.Remove(StartedHandle);
	FComboGraphDelegates::OnComboGraphEnded.Remove(EndedHandle);
	RemoveBindings();
	for (FExecutionRecord& Record : Executions) ClearRecord(Record, TEXT("Buffering stopped"));
	Executions.Reset();
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(SamplingTimer);
	bBuffering = false;
}

void UBertaComboGraphInputBufferComponent::ClearBufferedInputs()
{
	for (FExecutionRecord& Record : Executions) ClearRecord(Record, TEXT("Explicit clear"));
}

void UBertaComboGraphInputBufferComponent::EndPlay(const EEndPlayReason::Type Reason) { StopBuffering(); Super::EndPlay(Reason); }
void UBertaComboGraphInputBufferComponent::BeginDestroy() { StopBuffering(); Super::BeginDestroy(); }

bool UBertaComboGraphInputBufferComponent::CanConsumeWithStockInputPath(const ETriggerEvent TriggerEvent)
{
	return TriggerEvent == ETriggerEvent::Triggered;
}

bool UBertaComboGraphInputBufferComponent::IsExpired(const float CapturedAt, const float Now, const float Duration)
{
	return Duration <= 0.0f || Now - CapturedAt > Duration;
}

bool UBertaComboGraphInputBufferComponent::IsTaskForOwner(const UComboGraphAbilityTask_StartGraph& Task) const
{
	return Task.GetAvatarActorFromActorInfo() == GetOwner();
}

FString UBertaComboGraphInputBufferComponent::ExecutionId(const FExecutionRecord* Record)
{
	return Record ? FString::Printf(TEXT("%s|%s"), *GetPathNameSafe(Record->Task.Get()), *GetPathNameSafe(Record->Graph.Get())) : TEXT("None");
}

void UBertaComboGraphInputBufferComponent::BroadcastDiagnostic(const EBertaComboGraphInputBufferEventType Type, const FExecutionRecord* Record, const UInputAction* Action, const TCHAR* Reason)
{
	FBertaComboGraphInputBufferEvent Event;
	Event.Type = Type;
	Event.ExecutionId = ExecutionId(Record);
	Event.InputActionPath = GetPathNameSafe(Action);
	Event.NodePath = Record ? GetPathNameSafe(Record->Node.Get()) : TEXT("None");
	Event.TimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Event.Reason = Reason;
	OnInputBufferDiagnostic.Broadcast(Event);
}

void UBertaComboGraphInputBufferComponent::HandleGraphStarted(const UComboGraphAbilityTask_StartGraph& Task, const UComboGraph& Graph)
{
	if (!bBuffering || !IsTaskForOwner(Task)) return;
	FExecutionRecord& Record = Executions.AddDefaulted_GetRef();
	Record.Task = const_cast<UComboGraphAbilityTask_StartGraph*>(&Task);
	Record.Graph = const_cast<UComboGraph*>(&Graph);
	Record.Node = Task.GetCurrentNode();
	Record.bWindowOpen = Task.IsComboWindowOpened();
	RebuildBindings();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SamplingTimer, this, &ThisClass::SampleExecutions, FMath::Clamp(ActiveSamplingIntervalSeconds, 0.01f, 1.0f), true);
	}
}

void UBertaComboGraphInputBufferComponent::HandleGraphEnded(const UComboGraphAbilityTask_StartGraph& Task, const UComboGraph& Graph)
{
	for (int32 Index = Executions.Num() - 1; Index >= 0; --Index)
	{
		if (Executions[Index].Task.Get() == &Task)
		{
			ClearRecord(Executions[Index], TEXT("Graph ended"));
			Executions.RemoveAt(Index);
		}
	}
	RebuildBindings();
	if (Executions.IsEmpty()) if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(SamplingTimer);
}

bool UBertaComboGraphInputBufferComponent::NodeAcceptsTriggeredAction(const UComboGraphNodeAnimBase* Node, const UInputAction* Action)
{
	if (!Node || !Action) return false;
	for (UComboGraphNodeBase* Child : Node->ChildrenNodes)
	{
		const TObjectPtr<UComboGraphEdge>* EdgePtr = Node->Edges.Find(Child);
		const UComboGraphEdge* Edge = EdgePtr ? EdgePtr->Get() : nullptr;
		if (Edge && Edge->TransitionInput == Action && CanConsumeWithStockInputPath(Edge->GetEnhancedInputTriggerEvent())) return true;
	}
	return false;
}

void UBertaComboGraphInputBufferComponent::HandleTriggeredInput(const FInputActionInstance& Instance)
{
	const UInputAction* Action = Instance.GetSourceAction();
	TArray<int32> Candidates;
	for (int32 Index = 0; Index < Executions.Num(); ++Index)
	{
		const FExecutionRecord& Record = Executions[Index];
		if (!Record.bWindowOpen && NodeAcceptsTriggeredAction(Record.Node.Get(), Action)) Candidates.Add(Index);
	}
	if (Candidates.Num() != 1)
	{
		BroadcastDiagnostic(EBertaComboGraphInputBufferEventType::Rejected, nullptr, Action, Candidates.IsEmpty() ? TEXT("No closed-window execution accepts this Triggered action") : TEXT("Action is ambiguous across active executions"));
		return;
	}
	FExecutionRecord& Record = Executions[Candidates[0]];
	FBufferedInput Input;
	Input.Action = Action;
	Input.Node = Record.Node;
	Input.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Record.Inputs.Add(MoveTemp(Input));
	const int32 Limit = FMath::Clamp(MaximumBufferedInputs, 1, 8);
	if (Record.Inputs.Num() > Limit)
	{
		const int32 RemoveCount = Record.Inputs.Num() - Limit;
		for (int32 Index = 0; Index < RemoveCount; ++Index)
		{
			BroadcastDiagnostic(EBertaComboGraphInputBufferEventType::Cleared, &Record, Record.Inputs[Index].Action.Get(), TEXT("Buffer capacity evicted oldest input"));
		}
		Record.Inputs.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
	BroadcastDiagnostic(EBertaComboGraphInputBufferEventType::Buffered, &Record, Action, TEXT("Triggered input captured while Combo Window was closed"));
}

void UBertaComboGraphInputBufferComponent::SampleExecutions()
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	bool bBindingsDirty = false;
	for (int32 Index = Executions.Num() - 1; Index >= 0; --Index)
	{
		FExecutionRecord& Record = Executions[Index];
		UComboGraphAbilityTask_StartGraph* Task = Record.Task.Get();
		if (!Task)
		{
			ClearRecord(Record, TEXT("Task became invalid"));
			Executions.RemoveAt(Index);
			bBindingsDirty = true;
			continue;
		}
		for (int32 InputIndex = Record.Inputs.Num() - 1; InputIndex >= 0; --InputIndex)
		{
			if (IsExpired(Record.Inputs[InputIndex].Timestamp, Now, BufferDurationSeconds))
			{
				BroadcastDiagnostic(EBertaComboGraphInputBufferEventType::Expired, &Record, Record.Inputs[InputIndex].Action.Get(), TEXT("Buffer duration elapsed"));
				Record.Inputs.RemoveAt(InputIndex);
			}
		}
		if (Task->GetCurrentNode() != Record.Node.Get())
		{
			ClearRecord(Record, TEXT("Current node changed"));
			Record.Node = Task->GetCurrentNode();
			Record.bConsumePending = false;
			bBindingsDirty = true;
		}
		const bool bNowOpen = Task->IsComboWindowOpened();
		if (bNowOpen && !Record.bWindowOpen)
		{
			// Let Combo Graph's direct Enhanced Input binding win this frame. This avoids
			// replaying a buffered input when the original Triggered input is still active.
			Record.bConsumePending = bConsumeOnComboWindowOpen;
		}
		else if (bNowOpen && Record.bConsumePending)
		{
			if (Task->GetQueuedNode())
			{
				ClearRecord(Record, TEXT("Combo Graph accepted direct input before buffered replay"));
			}
			else
			{
				ConsumeBufferedInput(Record, Now);
			}
			Record.bConsumePending = false;
		}
		else if (!bNowOpen)
		{
			Record.bConsumePending = false;
		}
		Record.bWindowOpen = bNowOpen;
	}
	if (bBindingsDirty) RebuildBindings();
	if (Executions.IsEmpty()) if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(SamplingTimer);
}

void UBertaComboGraphInputBufferComponent::ConsumeBufferedInput(FExecutionRecord& Record, const float Now)
{
	TArray<float> CaptureTimes;
	CaptureTimes.Reserve(Record.Inputs.Num());
	for (const FBufferedInput& Input : Record.Inputs) CaptureTimes.Add(Input.Timestamp);
	const int32 SelectedIndex = BertaComboGraphInputBufferRules::SelectOldestUnexpired(CaptureTimes, Now, BufferDurationSeconds);
	if (SelectedIndex == INDEX_NONE) return;
	const UInputAction* Action = Record.Inputs[SelectedIndex].Action.Get();
	if (!Action || Record.Inputs[SelectedIndex].Node.Get() != Record.Node.Get()) { ClearRecord(Record, TEXT("Buffered input no longer belongs to current node")); return; }
	UComboGraphGameplayTasksComponent* TasksComponent = GetOwner() ? GetOwner()->FindComponentByClass<UComboGraphGameplayTasksComponent>() : nullptr;
	if (!TasksComponent)
	{
		BroadcastDiagnostic(EBertaComboGraphInputBufferEventType::Rejected, &Record, Action, TEXT("Combo Graph replicated gameplay-event component is unavailable"));
		Record.Inputs.Reset();
		return;
	}
	FGameplayEventData Payload;
	Payload.EventTag = FComboGraphNativeTags::Get().Input;
	Payload.Instigator = GetOwner();
	Payload.Target = GetOwner();
	Payload.OptionalObject = Action;
	TasksComponent->SendGameplayEventReplicated(Payload.EventTag, Payload);
	BroadcastDiagnostic(EBertaComboGraphInputBufferEventType::Consumed, &Record, Action, TEXT("Delivered once through Combo Graph replicated gameplay-event path as Triggered"));
	Record.Inputs.Reset();
}

void UBertaComboGraphInputBufferComponent::ClearRecord(FExecutionRecord& Record, const TCHAR* Reason)
{
	if (!Record.Inputs.IsEmpty()) BroadcastDiagnostic(EBertaComboGraphInputBufferEventType::Cleared, &Record, Record.Inputs[0].Action.Get(), Reason);
	Record.Inputs.Reset();
}

void UBertaComboGraphInputBufferComponent::RemoveBindings()
{
	if (UEnhancedInputComponent* Input = BoundInputComponent.Get()) for (const uint32 Handle : InputBindingHandles) Input->RemoveBindingByHandle(Handle);
	InputBindingHandles.Reset();
}

void UBertaComboGraphInputBufferComponent::RebuildBindings()
{
	RemoveBindings();
	if (!BoundInputComponent.IsValid()) BoundInputComponent = GetOwner() ? GetOwner()->FindComponentByClass<UEnhancedInputComponent>() : nullptr;
	UEnhancedInputComponent* Input = BoundInputComponent.Get();
	if (!Input) return;
	TSet<TObjectPtr<const UInputAction>> Actions;
	for (const FExecutionRecord& Record : Executions)
	{
		const UComboGraphNodeAnimBase* Node = Record.Node.Get();
		if (!Node) continue;
		for (UComboGraphNodeBase* Child : Node->ChildrenNodes)
		{
			const TObjectPtr<UComboGraphEdge>* EdgePtr = Node->Edges.Find(Child);
			const UComboGraphEdge* Edge = EdgePtr ? EdgePtr->Get() : nullptr;
			if (Edge && Edge->TransitionInput && CanConsumeWithStockInputPath(Edge->GetEnhancedInputTriggerEvent())) Actions.Add(Edge->TransitionInput);
		}
	}
	for (const UInputAction* Action : Actions)
	{
		FEnhancedInputActionEventBinding& Binding = Input->BindAction(Action, ETriggerEvent::Triggered, this, &ThisClass::HandleTriggeredInput);
		InputBindingHandles.Add(Binding.GetHandle());
	}
}
