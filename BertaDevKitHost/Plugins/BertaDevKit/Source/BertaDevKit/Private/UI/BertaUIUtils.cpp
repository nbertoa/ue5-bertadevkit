// BertaUIUtils.cpp
#include "UI/BertaUIUtils.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Log/BertaDevKitLog.h"

namespace
{
	APlayerController* ResolvePlayerController(const UObject* WorldContextObject,
	                                           APlayerController* PlayerController)
	{
		return IsValid(PlayerController)
			? PlayerController
			: UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	}
}

UUserWidget* UBertaUIUtils::CreateAndAddWidget(const UObject* WorldContextObject,
	                                              const TSubclassOf<UUserWidget> WidgetClass,
	                                              APlayerController* OwningPlayer,
	                                              const int32 ZOrder)
{
	if (!WidgetClass)
	{
		UE_LOG(LogBertaDevKit, Warning,
		       TEXT("[BertaUIUtils::CreateAndAddWidget] WidgetClass is null - returning null."));
		return nullptr;
	}

	APlayerController* PlayerController = ResolvePlayerController(WorldContextObject, OwningPlayer);
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogBertaDevKit, Warning,
		       TEXT("[BertaUIUtils::CreateAndAddWidget] No valid owning player or local player controller was available - returning null."));
		return nullptr;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
	if (!IsValid(Widget))
	{
		UE_LOG(LogBertaDevKit, Warning,
		       TEXT("[BertaUIUtils::CreateAndAddWidget] Failed to create widget '%s' - returning null."),
		       *GetNameSafe(WidgetClass));
		return nullptr;
	}

	Widget->AddToViewport(ZOrder);
	return Widget;
}

void UBertaUIUtils::SetPlayerInputMode(const UObject* WorldContextObject,
	                                      const EBertaPlayerInputMode Mode,
	                                      const bool bShowMouseCursor,
	                                      APlayerController* PlayerController)
{
	PlayerController = ResolvePlayerController(WorldContextObject, PlayerController);
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogBertaDevKit, Warning,
		       TEXT("[BertaUIUtils::SetPlayerInputMode] No valid player controller or local player controller was available."));
		return;
	}

	switch (Mode)
	{
	case EBertaPlayerInputMode::GameOnly:
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController, false);
		break;

	case EBertaPlayerInputMode::UIOnly:
		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PlayerController, nullptr, EMouseLockMode::DoNotLock, false);
		break;

	default:
		UE_LOG(LogBertaDevKit, Warning,
		       TEXT("[BertaUIUtils::SetPlayerInputMode] Unsupported input mode '%d'."),
		       static_cast<uint8>(Mode));
		return;
	}

	PlayerController->bShowMouseCursor = bShowMouseCursor;
}
