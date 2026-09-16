#pragma once

#include "BertaBlackEyeRevealSettings.h"
#include "Engine/DataAsset.h"
#include "BertaBlackEyeRevealPreset.generated.h"

/** Reusable reveal feel; the camera and activation policy belong to each consumer. */
UCLASS(BlueprintType)
class BERTABLACKEYECAMERAEXT_API UBertaBlackEyeRevealPreset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Copied by each accepted reveal session; changing this asset affects future sessions only. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Berta Black Eye Camera|Reveal")
    FBertaBlackEyeRevealSettings Settings;
};
