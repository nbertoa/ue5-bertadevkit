#pragma once

#include "UObject/Interface.h"
#include "BertaCinematicParticipant.generated.h"

/** A participant decides how it handles overlapping pause sources. */
UINTERFACE(BlueprintType)
class BERTABLACKEYECAMERAEXT_API UBertaCinematicParticipant : public UInterface
{
    GENERATED_BODY()
};

class BERTABLACKEYECAMERAEXT_API IBertaCinematicParticipant
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Berta Black Eye Camera|Participants")
    void OnCinematicPause(UObject* Source);
    virtual void OnCinematicPause_Implementation(UObject* Source) {}

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Berta Black Eye Camera|Participants")
    void OnCinematicResume(UObject* Source);
    virtual void OnCinematicResume_Implementation(UObject* Source) {}
};
