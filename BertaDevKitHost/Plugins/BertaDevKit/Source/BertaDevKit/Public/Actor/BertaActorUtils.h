// BertaActorUtils.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaActorUtils.generated.h"

class AActor;
class ACharacter;
class APawn;
class APlayerCameraManager;
class APlayerController;
class UActorComponent;
class UAnimInstance;
class UBlackboardComponent;

/** Blueprint conveniences for traversing common Actor relationships. */
UCLASS()
class BERTADEVKIT_API UBertaActorUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns Pawn's controller when it is a PlayerController, or null otherwise. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static APlayerController* GetPlayerControllerFromPawn(const APawn* Pawn);

	/** Returns Pawn's PlayerController when it is local, or null otherwise. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static APlayerController* GetLocalPlayerControllerFromPawn(const APawn* Pawn);

	/** Returns the camera manager owned by Pawn's PlayerController, or null when unavailable. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static APlayerCameraManager* GetPlayerCameraManagerFromPawn(const APawn* Pawn);

	/** Returns the blackboard owned by Pawn's AIController, or null when unavailable. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static UBlackboardComponent* GetBlackboardComponentFromPawn(const APawn* Pawn);

	/** Returns Character's current animation instance, or null when its mesh or animation instance is unavailable. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static UAnimInstance* GetAnimInstanceFromCharacter(const ACharacter* Character);

	/** Returns Component's owner when it is a Pawn, or null otherwise. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static APawn* GetOwnerPawn(const UActorComponent* Component);

	/** Returns Component's owner when it is a Character, or null otherwise. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static ACharacter* GetOwnerCharacter(const UActorComponent* Component);

	/** Returns Actor's instigator when it is a Character, or null otherwise. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static ACharacter* GetInstigatorCharacter(const AActor* Actor);

	/** Returns Actor's instigator controller when it is a PlayerController, or null otherwise. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Actor|Traversal")
	static APlayerController* GetInstigatorPlayerController(const AActor* Actor);
};
