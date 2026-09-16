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
    /** Source is the reveal component that owns this pause request; projects decide how pauses combine. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Berta Black Eye Camera|Participants")
    void OnCinematicPause(UObject* Source);
    virtual void OnCinematicPause_Implementation(UObject* Source) {}

    /** Called only after this Source actually notified the participant and while it is still valid. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Berta Black Eye Camera|Participants")
    void OnCinematicResume(UObject* Source);
    virtual void OnCinematicResume_Implementation(UObject* Source) {}
};
