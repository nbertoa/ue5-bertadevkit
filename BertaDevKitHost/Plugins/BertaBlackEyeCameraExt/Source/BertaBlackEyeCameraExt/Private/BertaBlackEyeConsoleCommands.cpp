#if !UE_BUILD_SHIPPING

#include "BertaBlackEyeCameraDebugLibrary.h"
#include "BertaBlackEyeCameraSwitcherComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
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
}

#endif
