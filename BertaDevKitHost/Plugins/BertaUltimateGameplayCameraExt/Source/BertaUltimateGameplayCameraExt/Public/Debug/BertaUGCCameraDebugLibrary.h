#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaUGCCameraDebugLibrary.generated.h"

class APlayerController;

/** A compact, read-only snapshot of the locally controlled UGC camera. */
UCLASS()
class BERTAULTIMATEGAMEPLAYCAMERAEXT_API UBertaUGCCameraDebugLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns false for a non-local controller, a non-UGC manager, or in Shipping. */
	UFUNCTION(BlueprintCallable, Category = "BertaUltimateGameplayCameraExt|Debug", meta = (ReturnDisplayName = "Success"))
	static bool GetUGCCameraDebugSummary(APlayerController* PlayerController, FString& OutSummary);
};
