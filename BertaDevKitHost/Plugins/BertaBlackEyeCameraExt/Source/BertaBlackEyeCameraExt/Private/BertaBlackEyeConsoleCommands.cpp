#if !UE_BUILD_SHIPPING

#include "Actors/BlackEyeCineCameraActorBase.h"
#include "BertaBlackEyeCameraDebugLibrary.h"
#include "BertaBlackEyeCameraRevealComponent.h"
#include "BertaBlackEyeCameraSwitcherComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "String/LexFromString.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaBlackEyeConsole, Log, All);

namespace
{
bool ParseIndex(const FString& Text, int32& OutIndex)
{
    return LexTryParseString(OutIndex, *Text) && OutIndex >= 0;
}

APlayerController* ResolveController(UWorld* World, const TArray<FString>& Args, int32 IndexArg)
{
    if (!IsValid(World) || !World->IsGameWorld() || !IsValid(World->GetGameInstance()))
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("Berta.BlackEye requires a game world with GameInstance."));
        return nullptr;
    }

    const TArray<ULocalPlayer*>& Players = World->GetGameInstance()->GetLocalPlayers();
    if (Players.IsEmpty())
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("Berta.BlackEye found no local players."));
        return nullptr;
    }

    int32 LocalIndex = 0;
    if (Args.IsValidIndex(IndexArg))
    {
        if (!ParseIndex(Args[IndexArg], LocalIndex) || !Players.IsValidIndex(LocalIndex))
        {
            UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("Invalid LocalPlayerIndex '%s'; available: 0..%d."),
                *Args[IndexArg], Players.Num() - 1);
            return nullptr;
        }
    }
    else if (Players.Num() != 1)
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("%d local players exist; specify LocalPlayerIndex."), Players.Num());
        return nullptr;
    }

    APlayerController* Controller = IsValid(Players[LocalIndex])
        ? Players[LocalIndex]->GetPlayerController(World) : nullptr;
    if (!IsValid(Controller) || !Controller->IsLocalPlayerController() || Controller->GetWorld() != World)
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("Local player %d has no usable controller in this world."), LocalIndex);
        return nullptr;
    }
    return Controller;
}

UBertaBlackEyeCameraSwitcherComponent* FindSwitcher(APlayerController* Controller)
{
    UBertaBlackEyeCameraSwitcherComponent* Switcher = Controller->FindComponentByClass<UBertaBlackEyeCameraSwitcherComponent>();
    if (!IsValid(Switcher))
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("Controller %s has no Berta Black Eye Camera Switcher Component."),
            *GetNameSafe(Controller));
    }
    return Switcher;
}

void Cycle(const TArray<FString>& Args, UWorld* World, bool bNext)
{
    if (Args.Num() > 1)
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("Usage: Berta.BlackEye.%s [LocalPlayerIndex]"),
            bNext ? TEXT("Next") : TEXT("Previous"));
        return;
    }
    APlayerController* Controller = ResolveController(World, Args, 0);
    if (!Controller) return;
    UBertaBlackEyeCameraSwitcherComponent* Switcher = FindSwitcher(Controller);
    if (!Switcher) return;

    const bool bSuccess = bNext ? Switcher->SelectNextCamera() : Switcher->SelectPreviousCamera();
    UE_LOG(LogBertaBlackEyeConsole, Display, TEXT("Berta.BlackEye.%s controller=%s success=%s selected=%d"),
        bNext ? TEXT("Next") : TEXT("Previous"), *GetNameSafe(Controller),
        bSuccess ? TEXT("true") : TEXT("false"), Switcher->GetSelectedCameraIndex());
}

void Next(const TArray<FString>& Args, UWorld* World) { Cycle(Args, World, true); }
void Previous(const TArray<FString>& Args, UWorld* World) { Cycle(Args, World, false); }

void Select(const TArray<FString>& Args, UWorld* World)
{
    int32 CameraIndex = INDEX_NONE;
    if (Args.Num() < 1 || Args.Num() > 2 || !ParseIndex(Args[0], CameraIndex))
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning,
            TEXT("Usage: Berta.BlackEye.Select <CameraIndex> [LocalPlayerIndex]"));
        return;
    }
    APlayerController* Controller = ResolveController(World, Args, 1);
    if (!Controller) return;
    UBertaBlackEyeCameraSwitcherComponent* Switcher = FindSwitcher(Controller);
    if (!Switcher) return;
    const bool bSuccess = Switcher->SelectCamera(CameraIndex);
    UE_LOG(LogBertaBlackEyeConsole, Display, TEXT("Berta.BlackEye.Select controller=%s index=%d success=%s"),
        *GetNameSafe(Controller), CameraIndex, bSuccess ? TEXT("true") : TEXT("false"));
}

void Dump(const TArray<FString>& Args, UWorld* World)
{
    if (Args.Num() > 1)
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("Usage: Berta.BlackEye.Dump [LocalPlayerIndex]"));
        return;
    }
    APlayerController* Controller = ResolveController(World, Args, 0);
    if (!Controller) return;
    FString Summary;
    if (UBertaBlackEyeCameraDebugLibrary::GetBlackEyeCameraDebugSummary(Controller, Summary))
    {
        UE_LOG(LogBertaBlackEyeConsole, Display, TEXT("Berta.BlackEye.Dump\n%s"), *Summary);
    }
    else
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning, TEXT("Berta.BlackEye.Dump: %s"), *Summary);
    }
}

void ReplayLastReveal(const TArray<FString>& Args, UWorld* World)
{
    if (Args.Num() > 1)
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning,
            TEXT("Usage: Berta.BlackEye.ReplayLastReveal [LocalPlayerIndex]"));
        return;
    }
    APlayerController* Controller = ResolveController(World, Args, 0);
    if (!Controller) return;

    UBertaBlackEyeCameraRevealComponent* Latest = nullptr;
    double LatestStart = 0.0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UBertaBlackEyeCameraRevealComponent*> Components;
        It->GetComponents(Components);
        for (UBertaBlackEyeCameraRevealComponent* Reveal : Components)
        {
            if (IsValid(Reveal) && Reveal->GetLastRevealController() == Controller &&
                Reveal->GetLastSuccessfulRevealStartRealTime() > LatestStart)
            {
                Latest = Reveal;
                LatestStart = Reveal->GetLastSuccessfulRevealStartRealTime();
            }
        }
    }
    if (!Latest)
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning,
            TEXT("No replayable reveal component found for local controller %s (destroyed components and previous worlds are not retained)."),
            *GetNameSafe(Controller));
        return;
    }
    if (Latest->IsRevealActive())
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning,
            TEXT("Last reveal component %s is already active; replay rejected without interrupting it."),
            *GetNameSafe(Latest));
        return;
    }
    if (!IsValid(Latest->TargetCamera))
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning,
            TEXT("Last reveal component %s has no valid TargetCamera; replay rejected."), *GetNameSafe(Latest));
        return;
    }

    const EBertaBlackEyeRevealDurationMode Mode = Latest->GetLastRevealStartMode();
    UE_LOG(LogBertaBlackEyeConsole, Display,
        TEXT("Replaying component %s (owner=%s, last start %.1fs ago, mode=%s) with its current configuration."),
        *GetNameSafe(Latest), *GetNameSafe(Latest->GetOwner()), FPlatformTime::Seconds() - LatestStart,
        Mode == EBertaBlackEyeRevealDurationMode::Manual ? TEXT("Manual") : TEXT("Timed"));
    if (!Latest->StartCameraRevealForController(Controller, Mode))
    {
        UE_LOG(LogBertaBlackEyeConsole, Warning,
            TEXT("Replay start was rejected by component %s; inspect its TargetCamera, timing, controller, and lifecycle state."),
            *GetNameSafe(Latest));
        return;
    }
    if (Mode == EBertaBlackEyeRevealDurationMode::Manual)
    {
        UE_LOG(LogBertaBlackEyeConsole, Display,
            TEXT("Manual replay started; call StopCameraReveal on component %s to finish it."), *GetNameSafe(Latest));
    }
}

FAutoConsoleCommandWithWorldAndArgs NextCommand(TEXT("Berta.BlackEye.Next"),
    TEXT("Select next camera on the local controller switcher; optional LocalPlayerIndex."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Next));
FAutoConsoleCommandWithWorldAndArgs PreviousCommand(TEXT("Berta.BlackEye.Previous"),
    TEXT("Select previous camera on the local controller switcher; optional LocalPlayerIndex."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Previous));
FAutoConsoleCommandWithWorldAndArgs SelectCommand(TEXT("Berta.BlackEye.Select"),
    TEXT("Select <CameraIndex> [LocalPlayerIndex] on the local controller switcher."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Select));
FAutoConsoleCommandWithWorldAndArgs DumpCommand(TEXT("Berta.BlackEye.Dump"),
    TEXT("Print local Black Eye view target and reveal state; optional LocalPlayerIndex."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Dump));
FAutoConsoleCommandWithWorldAndArgs ReplayCommand(TEXT("Berta.BlackEye.ReplayLastReveal"),
    TEXT("Re-run the last successful reveal component for the local player using its current configuration."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ReplayLastReveal));
}

#endif
