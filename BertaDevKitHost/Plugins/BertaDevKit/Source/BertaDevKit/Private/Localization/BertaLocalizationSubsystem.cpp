#include "Localization/BertaLocalizationSubsystem.h"

#include "Async/Async.h"
#include "CoreGlobals.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Misc/AssertionMacros.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/WeakObjectPtrTemplates.h"

void UBertaLocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	check(IsInGameThread());
	check(!CultureChangedHandle.IsValid());

	FInternationalization& Internationalization = FInternationalization::Get();
	CachedState = { Internationalization.GetCurrentLanguage()->GetName(), Internationalization.GetCurrentLocale()->GetName() };
	CultureChangedHandle = Internationalization.OnCultureChanged().AddUObject(this, &ThisClass::HandleCultureChanged);
}

void UBertaLocalizationSubsystem::Deinitialize()
{
	check(IsInGameThread());
	// Do not recreate internationalization if it has already been torn down during shutdown.
	if (FInternationalization::IsAvailable())
	{
		FInternationalization::Get().OnCultureChanged().Remove(CultureChangedHandle);
	}
	CultureChangedHandle.Reset();
	PendingStates.Reset();
	CachedState = {};
	Super::Deinitialize();
}

void UBertaLocalizationSubsystem::HandleCultureChanged()
{
	if (!IsInGameThread())
	{
		const TWeakObjectPtr<UBertaLocalizationSubsystem> WeakThis(this);
		AsyncTask(ENamedThreads::GameThread, [WeakThis]()
		{
			if (UBertaLocalizationSubsystem* Subsystem = WeakThis.Get())
			{
				if (Subsystem->CultureChangedHandle.IsValid())
				{
					// Observe the latest native state; an older queued snapshot could reverse a newer transition.
					Subsystem->HandleCultureChanged();
				}
			}
		});
		return;
	}
	FInternationalization& Internationalization = FInternationalization::Get();
	const FCultureState State = { Internationalization.GetCurrentLanguage()->GetName(), Internationalization.GetCurrentLocale()->GetName() };
	DispatchCultureState(State);
}

void UBertaLocalizationSubsystem::DispatchCultureState(const FCultureState& State)
{
	check(IsInGameThread());
	// A queued notification may arrive after Deinitialize, while the UObject still exists.
	if (!CultureChangedHandle.IsValid())
	{
		return;
	}
	PendingStates.Add(State);
	if (bIsBroadcasting)
	{
		return;
	}

	// A listener may change language/locale synchronously. Finish this pair before dispatching that change.
	bIsBroadcasting = true;
	for (int32 Index = 0; Index < PendingStates.Num() && CultureChangedHandle.IsValid(); ++Index)
	{
		const FCultureState Previous = CachedState;
		const FCultureState Current = PendingStates[Index];
		CachedState = Current;
		if (Previous.Language != Current.Language)
		{
			OnLanguageChanged.Broadcast(Previous.Language, Current.Language);
		}
		if (CultureChangedHandle.IsValid() && Previous.Locale != Current.Locale)
		{
			OnLocaleChanged.Broadcast(Previous.Locale, Current.Locale);
		}
	}
	PendingStates.Reset();
	bIsBroadcasting = false;
}
