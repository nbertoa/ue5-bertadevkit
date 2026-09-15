#include "BertaUltimateGameplayCameraExt.h"

#if !UE_BUILD_SHIPPING

#include "Components/BertaUGCCameraCycleComponent.h"
#include "Debug/BertaUGCCameraDebugLibrary.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "String/LexFromString.h"

namespace
{
	bool ParseNonNegativeIndex(const FString& Value, int32& OutIndex)
	{
		return LexTryParseString(OutIndex, *Value) && OutIndex >= 0;
	}

	APlayerController* ResolveLocalController(UWorld* World, const TArray<FString>& Args, int32 PlayerArgIndex)
	{
		if (!World || !World->GetGameInstance())
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Berta.UGC command requires a game world with a GameInstance."));
			return nullptr;
		}
		const TArray<ULocalPlayer*>& Players = World->GetGameInstance()->GetLocalPlayers();
		if (Players.IsEmpty())
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Berta.UGC command found no local players in world %s."), *GetNameSafe(World));
			return nullptr;
		}
		int32 PlayerIndex = 0;
		if (Args.IsValidIndex(PlayerArgIndex))
		{
			if (!ParseNonNegativeIndex(Args[PlayerArgIndex], PlayerIndex) || !Players.IsValidIndex(PlayerIndex))
			{
				UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Berta.UGC invalid local-player index '%s' (available: 0..%d)."),
					*Args[PlayerArgIndex], Players.Num() - 1);
				return nullptr;
			}
		}
		else if (Players.Num() != 1)
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning,
				TEXT("Berta.UGC world %s has %d local players; specify LocalPlayerIndex."), *GetNameSafe(World), Players.Num());
			return nullptr;
		}
		APlayerController* Controller = IsValid(Players[PlayerIndex]) ? Players[PlayerIndex]->GetPlayerController(World) : nullptr;
		if (!IsValid(Controller) || Controller->GetWorld() != World || !Controller->IsLocalController())
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning,
				TEXT("Berta.UGC local player %d has no valid controller in world %s."), PlayerIndex, *GetNameSafe(World));
			return nullptr;
		}
		return Controller;
	}

	void Cycle(const TArray<FString>& Args, UWorld* World, bool bNext)
	{
		if (Args.Num() > 1)
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Usage: Berta.UGC.%s [LocalPlayerIndex]"), bNext ? TEXT("Next") : TEXT("Previous"));
			return;
		}
		APlayerController* Controller = ResolveLocalController(World, Args, 0);
		if (!Controller) return;
		UBertaUGCCameraCycleComponent* Component = Controller->FindComponentByClass<UBertaUGCCameraCycleComponent>();
		if (!IsValid(Component))
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Berta.UGC controller %s has no camera cycle component."), *GetNameSafe(Controller));
			return;
		}
		const bool bSuccess = bNext ? Component->SelectNextCamera() : Component->SelectPreviousCamera();
		UE_LOG(LogBertaUltimateGameplayCameraExt, Display, TEXT("Berta.UGC.%s controller=%s success=%s storedCycleIndex=%d"),
			bNext ? TEXT("Next") : TEXT("Previous"), *GetNameSafe(Controller), bSuccess ? TEXT("true") : TEXT("false"), Component->CurrentCameraIndex);
	}

	void Next(const TArray<FString>& Args, UWorld* World) { Cycle(Args, World, true); }
	void Previous(const TArray<FString>& Args, UWorld* World) { Cycle(Args, World, false); }

	void Select(const TArray<FString>& Args, UWorld* World)
	{
		int32 CameraIndex = INDEX_NONE;
		if (Args.Num() < 1 || Args.Num() > 2 || !ParseNonNegativeIndex(Args[0], CameraIndex))
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Usage: Berta.UGC.Select <CameraIndex> [LocalPlayerIndex]"));
			return;
		}
		APlayerController* Controller = ResolveLocalController(World, Args, 1);
		if (!Controller) return;
		UBertaUGCCameraCycleComponent* Component = Controller->FindComponentByClass<UBertaUGCCameraCycleComponent>();
		if (!IsValid(Component))
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Berta.UGC controller %s has no camera cycle component."), *GetNameSafe(Controller));
			return;
		}
		const bool bSuccess = Component->SelectCamera(CameraIndex);
		UE_LOG(LogBertaUltimateGameplayCameraExt, Display, TEXT("Berta.UGC.Select controller=%s requested=%d success=%s storedCycleIndex=%d"),
			*GetNameSafe(Controller), CameraIndex, bSuccess ? TEXT("true") : TEXT("false"), Component->CurrentCameraIndex);
	}

	void Dump(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() > 1)
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Usage: Berta.UGC.Dump [LocalPlayerIndex]"));
			return;
		}
		APlayerController* Controller = ResolveLocalController(World, Args, 0);
		if (!Controller) return;
		FString Summary;
		if (UBertaUGCCameraDebugLibrary::GetUGCCameraDebugSummary(Controller, Summary))
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Display, TEXT("Berta.UGC.Dump\n%s"), *Summary);
		}
		else
		{
			UE_LOG(LogBertaUltimateGameplayCameraExt, Warning, TEXT("Berta.UGC.Dump requires a valid UGC camera manager on controller %s."), *GetNameSafe(Controller));
		}
	}

	FAutoConsoleCommandWithWorldAndArgs NextCommand(TEXT("Berta.UGC.Next"),
		TEXT("Select the next Berta UGC cycle preset. Optional LocalPlayerIndex; required for split screen."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Next));
	FAutoConsoleCommandWithWorldAndArgs PreviousCommand(TEXT("Berta.UGC.Previous"),
		TEXT("Select the previous Berta UGC cycle preset. Optional LocalPlayerIndex; required for split screen."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Previous));
	FAutoConsoleCommandWithWorldAndArgs SelectCommand(TEXT("Berta.UGC.Select"),
		TEXT("Select Berta UGC cycle preset <CameraIndex> [LocalPlayerIndex]."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Select));
	FAutoConsoleCommandWithWorldAndArgs DumpCommand(TEXT("Berta.UGC.Dump"),
		TEXT("Print a UGC camera summary. Optional LocalPlayerIndex; required for split screen."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Dump));
}

#endif
