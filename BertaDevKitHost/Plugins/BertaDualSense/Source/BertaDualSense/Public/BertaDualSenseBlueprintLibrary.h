#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaDualSenseBlueprintLibrary.generated.h"

UCLASS()
class BERTADUALSENSE_API UBertaDualSenseBlueprintLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaDualSense|Output")
	static bool SetMicrophoneLed(int32 ControllerId, bool bEnabled);
};
