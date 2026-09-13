// BertaComponentUtils.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaComponentUtils.generated.h"

class UActorComponent;
class USceneComponent;

/** Blueprint conveniences for inspecting component state. */
UCLASS()
class BERTADEVKIT_API UBertaComponentUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns a human-readable snapshot of a scene component's attachment and transform state. */
	UFUNCTION(BlueprintPure,
		Category = "BertaDevKit|Component|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get Attachment Debug Summary"))
	static FString GetAttachmentDebugSummary(const USceneComponent* Component);

	/** Returns a human-readable snapshot of an actor component's lifecycle and tick state. */
	UFUNCTION(BlueprintPure,
		Category = "BertaDevKit|Component|Debug",
		meta = (DevelopmentOnly, DisplayName = "Get Component Lifecycle Debug Summary"))
	static FString GetLifecycleDebugSummary(const UActorComponent* Component);
};
