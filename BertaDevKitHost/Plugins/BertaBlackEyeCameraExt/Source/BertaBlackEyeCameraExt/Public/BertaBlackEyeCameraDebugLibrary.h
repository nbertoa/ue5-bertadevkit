#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaBlackEyeCameraDebugLibrary.generated.h"

class APlayerController;

/** On-demand local camera diagnostics; unavailable in Shipping. */
UCLASS()
class BERTABLACKEYECAMERAEXT_API UBertaBlackEyeCameraDebugLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** False with explanatory OutSummary for invalid local controllers or Shipping; never changes camera state. */
    UFUNCTION(BlueprintCallable, Category = "Berta Black Eye Camera|Debug")
    static bool GetBlackEyeCameraDebugSummary(APlayerController* PlayerController, FString& OutSummary);
};
