#include "BertaBlackEyeRevealComponentVisualizer.h"

#include "BertaBlackEyeCameraRevealComponent.h"
#include "BertaBlackEyeCameraTrigger.h"
#include "BertaBlackEyeRevealPreset.h"
#include "Actors/BlackEyeCineCameraActorBase.h"
#include "CanvasTypes.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "PrimitiveDrawInterface.h"

namespace
{
FVector GetRevealAnchor(const AActor* Owner)
{
    if (const ABertaBlackEyeCameraTrigger* Trigger = Cast<ABertaBlackEyeCameraTrigger>(Owner))
    {
        if (IsValid(Trigger->BoxCollision)) return Trigger->BoxCollision->GetComponentLocation();
    }
    return Owner->GetActorLocation();
}

int32 GetValidParticipantCount(const UBertaBlackEyeCameraRevealComponent* Reveal)
{
    TSet<const AActor*> Seen;
    for (const AActor* Participant : Reveal->Participants)
    {
        if (IsValid(Participant)) Seen.Add(Participant);
    }
    return Seen.Num();
}
}

void FBertaBlackEyeRevealComponentVisualizer::DrawVisualization(const UActorComponent* Component,
    const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    const UBertaBlackEyeCameraRevealComponent* Reveal = Cast<UBertaBlackEyeCameraRevealComponent>(Component);
    const AActor* Owner = IsValid(Reveal) ? Reveal->GetOwner() : nullptr;
    if (!IsValid(Owner) || !PDI) return;

    const FVector Anchor = GetRevealAnchor(Owner);
    if (IsValid(Reveal->TargetCamera))
    {
        PDI->DrawLine(Anchor, Reveal->TargetCamera->GetActorLocation(), FLinearColor::Yellow, SDPG_World, 3.0f);
    }
    TSet<const AActor*> Seen;
    for (const AActor* Participant : Reveal->Participants)
    {
        if (!IsValid(Participant) || Seen.Contains(Participant)) continue;
        Seen.Add(Participant);
        PDI->DrawLine(Anchor, Participant->GetActorLocation(), FLinearColor(0.0f, 0.75f, 1.0f), SDPG_World, 1.5f);
    }
}

void FBertaBlackEyeRevealComponentVisualizer::DrawVisualizationHUD(const UActorComponent* Component,
    const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
    const UBertaBlackEyeCameraRevealComponent* Reveal = Cast<UBertaBlackEyeCameraRevealComponent>(Component);
    const AActor* Owner = IsValid(Reveal) ? Reveal->GetOwner() : nullptr;
    if (!IsValid(Owner) || !Canvas || !GEngine) return;

    TArray<FString> Lines;
    Lines.Add(TEXT("Black Eye Reveal"));
    Lines.Add(FString::Printf(TEXT("Target: %s"), IsValid(Reveal->TargetCamera)
        ? *GetNameSafe(Reveal->TargetCamera) : TEXT("None")));
    Lines.Add(FString::Printf(TEXT("Preset: %s"), IsValid(Reveal->RevealPreset)
        ? *GetNameSafe(Reveal->RevealPreset) : TEXT("Inline")));
    Lines.Add(FString::Printf(TEXT("Return: %s"),
        *StaticEnum<EBertaBlackEyeReturnTargetPolicy>()->GetNameStringByValue(static_cast<int64>(Reveal->ReturnTargetPolicy))));
    Lines.Add(FString::Printf(TEXT("External: %s"),
        *StaticEnum<EBertaBlackEyeExternalCameraChangePolicy>()->GetNameStringByValue(static_cast<int64>(Reveal->ExternalCameraChangePolicy))));
    Lines.Add(FString::Printf(TEXT("Participants: %d"), GetValidParticipantCount(Reveal)));
    if (const ABertaBlackEyeCameraTrigger* Trigger = Cast<ABertaBlackEyeCameraTrigger>(Owner))
    {
        Lines.Add(FString::Printf(TEXT("Trigger: %s  Enabled: %s  Once: %s"),
            *StaticEnum<EBertaBlackEyeRevealEndMode>()->GetNameStringByValue(static_cast<int64>(Trigger->EndMode)),
            Trigger->bEnabled ? TEXT("true") : TEXT("false"), Trigger->bTriggerOnce ? TEXT("true") : TEXT("false")));
    }

    double Y = 90.0;
    for (const FString& Line : Lines)
    {
        Canvas->DrawShadowedString(16.0, Y, Line, GEngine->GetSmallFont(), FLinearColor::White);
        Y += 18.0;
    }
}
