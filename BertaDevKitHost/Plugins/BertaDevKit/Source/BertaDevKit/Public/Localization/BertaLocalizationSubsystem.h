#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "Subsystems/EngineSubsystem.h"

#include "BertaLocalizationSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBertaLanguageChangedEvent,
	const FString&, PreviousLanguage, const FString&, CurrentLanguage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBertaLocaleChangedEvent,
	const FString&, PreviousLocale, const FString&, CurrentLocale);

/** Process-wide notifications on the game thread, shared across worlds and PIE. */
UCLASS(BlueprintType)
class BERTADEVKIT_API UBertaLocalizationSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Text localization language changed. Initialization and asset-group-only changes do not emit events. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|Localization")
	FBertaLanguageChangedEvent OnLanguageChanged;

	/** Regional formatting locale changed, independently of text localization language. */
	UPROPERTY(BlueprintAssignable, Category = "BertaDevKit|Localization")
	FBertaLocaleChangedEvent OnLocaleChanged;

private:
	void HandleCultureChanged();

	struct FCultureState
	{
		FString Language;
		FString Locale;
	};

	void DispatchCultureState(const FCultureState& State);

	FCultureState CachedState;
	TArray<FCultureState> PendingStates;
	FDelegateHandle CultureChangedHandle;
	bool bIsBroadcasting = false;
};
