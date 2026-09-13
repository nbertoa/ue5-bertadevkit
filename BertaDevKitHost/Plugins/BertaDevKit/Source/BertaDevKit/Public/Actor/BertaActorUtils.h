// BertaActorUtils.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaActorUtils.generated.h"

class AActor;
class ACharacter;
class APawn;
class APlayerController;
class UAnimInstance;

/** Blueprint conveniences for traversing common Actor relationships. */
UCLASS()
class BERTADEVKIT_API UBertaActorUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns Pawn's controller when it is a PlayerController, or null otherwise. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static APlayerController* GetPlayerControllerFromPawn(const APawn* Pawn);

	/** Returns Character's current animation instance, or null when its mesh or animation instance is unavailable. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static UAnimInstance* GetAnimInstanceFromCharacter(const ACharacter* Character);

	/** Returns Actor's instigator controller when it is a PlayerController, or null otherwise. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static APlayerController* GetInstigatorPlayerController(const AActor* Actor);
};
