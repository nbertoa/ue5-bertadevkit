// BertaUIUtils.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "BertaUIUtils.generated.h"

class APlayerController;

UENUM(BlueprintType)
enum class EBertaPlayerInputMode : uint8
{
	GameOnly UMETA(DisplayName = "Game Only"),
	UIOnly UMETA(DisplayName = "UI Only")
};

/** Blueprint conveniences for common UI and local-player input boilerplate. */
UCLASS()
class BERTADEVKIT_API UBertaUIUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Creates WidgetClass for OwningPlayer (or local player zero) and adds it to the viewport.
	 * Returns null when the class or player controller is unavailable.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|UI",
		meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "WidgetClass", DynamicOutputParam = "ReturnValue", AdvancedDisplay = "OwningPlayer,ZOrder", DisplayName = "Create And Add Widget", ReturnDisplayName = "Created Widget"))
	static UUserWidget* CreateAndAddWidget(const UObject* WorldContextObject,
	                                       TSubclassOf<UUserWidget> WidgetClass,
	                                       APlayerController* OwningPlayer = nullptr,
	                                       int32 ZOrder = 0);

	/**
	 * Applies a game-only or UI-only input mode to PlayerController (or local player zero)
	 * and sets its mouse cursor visibility. Does nothing when no valid controller is available.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|UI",
		meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "PlayerController", DisplayName = "Set Player Input Mode"))
	static void SetPlayerInputMode(const UObject* WorldContextObject,
	                               EBertaPlayerInputMode Mode,
	                               bool bShowMouseCursor,
	                               APlayerController* PlayerController = nullptr);
};
