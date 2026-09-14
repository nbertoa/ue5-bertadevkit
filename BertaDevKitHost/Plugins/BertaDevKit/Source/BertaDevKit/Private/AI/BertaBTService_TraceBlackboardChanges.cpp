#include "AI/BertaBTService_TraceBlackboardChanges.h"

#include "AI/BertaBlackboardDebugUtils.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Debug/BertaDebugLog.h"

UBertaBTService_TraceBlackboardChanges::UBertaBTService_TraceBlackboardChanges(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Trace Blackboard Changes");
	bCreateNodeInstance = true;
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
	bNotifyTick = false;
}

void UBertaBTService_TraceBlackboardChanges::OnBecomeRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UnregisterObservers();

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard || !Blackboard->HasValidAsset())
	{
		return;
	}

	TArray<FBlackboard::FKey> KeyIds;
	if (Keys.IsEmpty() && bTraceAllKeysWhenEmpty)
	{
		KeyIds.Reserve(Blackboard->GetNumKeys());
		for (int32 KeyIndex = 0; KeyIndex < Blackboard->GetNumKeys(); ++KeyIndex)
		{
			KeyIds.Add(static_cast<FBlackboard::FKey>(KeyIndex));
		}
	}
	else
	{
		for (const FName KeyName : Keys)
		{
			const FBlackboard::FKey KeyId = Blackboard->GetKeyID(KeyName);
			if (KeyId == FBlackboard::InvalidKey)
			{
				UE_LOG(
					LogBertaDebug,
					Warning,
					TEXT("Trace Blackboard Changes%s: key '%s' does not exist."),
					Label.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" [%s]"), *Label),
					*KeyName.ToString());
				continue;
			}

			KeyIds.AddUnique(KeyId);
		}
	}

	ObservedBlackboard = Blackboard;
	Registrations.Reserve(KeyIds.Num());
	for (const FBlackboard::FKey KeyId : KeyIds)
	{
		const FName KeyName = Blackboard->GetKeyName(KeyId);
		if (KeyName.IsNone())
		{
			continue;
		}

		FBlackboardObserverRegistration& Registration = Registrations.AddDefaulted_GetRef();
		Registration.KeyId = KeyId;
		Registration.KeyName = KeyName;
		Registration.Handle = Blackboard->RegisterObserver(
			KeyId,
			this,
			FOnBlackboardChangeNotification::CreateUObject(this, &ThisClass::HandleBlackboardChange));
	}
}

void UBertaBTService_TraceBlackboardChanges::OnCeaseRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UnregisterObservers();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBertaBTService_TraceBlackboardChanges::OnInstanceDestroyed(UBehaviorTreeComponent& OwnerComp)
{
	UnregisterObservers();
	Super::OnInstanceDestroyed(OwnerComp);
}

FString UBertaBTService_TraceBlackboardChanges::GetStaticDescription() const
{
	TArray<FString> KeyNames;
	KeyNames.Reserve(Keys.Num());
	for (const FName Key : Keys)
	{
		KeyNames.AddUnique(Key.ToString());
	}
	KeyNames.Sort();

	const FString KeyDescription = KeyNames.IsEmpty()
		? (bTraceAllKeysWhenEmpty ? TEXT("All") : TEXT("None"))
		: FString::Join(KeyNames, TEXT(", "));
	return FString::Printf(
		TEXT("%s\nKeys: %s\nLabel: %s"),
		*Super::GetStaticDescription(),
		*KeyDescription,
		Label.IsEmpty() ? TEXT("None") : *Label);
}

EBlackboardNotificationResult UBertaBTService_TraceBlackboardChanges::HandleBlackboardChange(
	const UBlackboardComponent& Blackboard,
	const FBlackboard::FKey ChangedKeyId)
{
	if (ObservedBlackboard.Get() != &Blackboard)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	const FName KeyName = Blackboard.GetKeyName(ChangedKeyId);
	FString ValueDescription;
	if (!UBertaBlackboardDebugUtils::GetKeyValueDescription(&Blackboard, KeyName, ValueDescription))
	{
		return EBlackboardNotificationResult::ContinueObserving;
	}

	if (Label.IsEmpty())
	{
		UE_LOG(LogBertaDebug, Log, TEXT("Blackboard %s = %s"), *KeyName.ToString(), *ValueDescription);
	}
	else
	{
		UE_LOG(
			LogBertaDebug,
			Log,
			TEXT("[%s] Blackboard %s = %s"),
			*Label,
			*KeyName.ToString(),
			*ValueDescription);
	}
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBertaBTService_TraceBlackboardChanges::UnregisterObservers()
{
	if (UBlackboardComponent* Blackboard = ObservedBlackboard.Get())
	{
		for (const FBlackboardObserverRegistration& Registration : Registrations)
		{
			if (Registration.KeyId != FBlackboard::InvalidKey && Registration.Handle.IsValid())
			{
				Blackboard->UnregisterObserver(Registration.KeyId, Registration.Handle);
			}
		}
	}

	Registrations.Reset();
	ObservedBlackboard.Reset();
}
