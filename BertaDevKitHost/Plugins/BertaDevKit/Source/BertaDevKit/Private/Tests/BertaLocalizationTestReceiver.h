#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "UObject/Object.h"

#include "BertaLocalizationTestReceiver.generated.h"

// UHT needs this private reflected receiver in every configuration; only automation uses it.
UCLASS(Transient, NotBlueprintable)
class UBertaLocalizationTestReceiver : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void RecordLanguage(const FString& PreviousLanguage, const FString& CurrentLanguage);

	UFUNCTION()
	void RecordLocale(const FString& PreviousLocale, const FString& CurrentLocale);

	TArray<FString> Events;
	FString ReentrantLocale;
};
