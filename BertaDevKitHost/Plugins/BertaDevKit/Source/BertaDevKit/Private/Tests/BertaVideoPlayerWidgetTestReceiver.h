#pragma once
#include "UObject/Object.h"
#include "BertaVideoPlayerWidgetTestReceiver.generated.h"

class UBertaVideoPlayerWidget;

UCLASS(Transient, NotBlueprintable)
class UBertaVideoPlayerWidgetTestReceiver : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void Started(UBertaVideoPlayerWidget* Widget);
	UFUNCTION()
	void Completed(UBertaVideoPlayerWidget* Widget);
	UFUNCTION()
	void Failed(UBertaVideoPlayerWidget* Widget, FString Error);
	int32 StartedCount = 0;
	int32 CompletedCount = 0;
	int32 FailedCount = 0;
	bool bClearOnEvent = false;
	bool bStopOnStarted = false;
};
