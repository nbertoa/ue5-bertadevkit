#pragma once

#include "UObject/Object.h"

#include "BertaImagePlayerWidgetTestReceiver.generated.h"

class UBertaImagePlayerWidget;
class UTexture2D;

// UHT needs this private reflected receiver in every configuration; only automation uses it.
UCLASS(Transient, NotBlueprintable)
class UBertaImagePlayerWidgetTestReceiver : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void RecordCompletion(UBertaImagePlayerWidget* Widget);

	int32 CompletionCount = 0;
	bool bPlayAcceptedDuringCompletion = false;
	bool bCloseOnCompletion = false;
	bool bReplaceOnCompletion = false;
	UPROPERTY()
	TObjectPtr<UTexture2D> ReplacementTexture;
};
