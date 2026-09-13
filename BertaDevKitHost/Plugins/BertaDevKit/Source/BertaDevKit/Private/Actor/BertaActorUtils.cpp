// BertaActorUtils.cpp
#include "Actor/BertaActorUtils.h"

#include "AIController.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

APlayerController* UBertaActorUtils::GetPlayerControllerFromPawn(const APawn* Pawn)
{
	return Pawn != nullptr ? Pawn->GetController<APlayerController>() : nullptr;
}

APlayerController* UBertaActorUtils::GetLocalPlayerControllerFromPawn(const APawn* Pawn)
{
	APlayerController* PlayerController = GetPlayerControllerFromPawn(Pawn);
	return PlayerController != nullptr && PlayerController->IsLocalController() ? PlayerController : nullptr;
}

APlayerCameraManager* UBertaActorUtils::GetPlayerCameraManagerFromPawn(const APawn* Pawn)
{
	const APlayerController* PlayerController = GetPlayerControllerFromPawn(Pawn);
	return PlayerController != nullptr ? PlayerController->PlayerCameraManager : nullptr;
}

UBlackboardComponent* UBertaActorUtils::GetBlackboardComponentFromPawn(const APawn* Pawn)
{
	AAIController* AIController = Pawn != nullptr ? Pawn->GetController<AAIController>() : nullptr;
	return AIController != nullptr ? AIController->GetBlackboardComponent() : nullptr;
}

UAnimInstance* UBertaActorUtils::GetAnimInstanceFromCharacter(const ACharacter* Character)
{
	const USkeletalMeshComponent* Mesh = Character != nullptr ? Character->GetMesh() : nullptr;
	return Mesh != nullptr ? Mesh->GetAnimInstance() : nullptr;
}

APawn* UBertaActorUtils::GetOwnerPawn(const UActorComponent* Component)
{
	return Component != nullptr ? Component->GetOwner<APawn>() : nullptr;
}

ACharacter* UBertaActorUtils::GetOwnerCharacter(const UActorComponent* Component)
{
	return Component != nullptr ? Component->GetOwner<ACharacter>() : nullptr;
}

ACharacter* UBertaActorUtils::GetInstigatorCharacter(const AActor* Actor)
{
	return Actor != nullptr ? Actor->GetInstigator<ACharacter>() : nullptr;
}

APlayerController* UBertaActorUtils::GetInstigatorPlayerController(const AActor* Actor)
{
	return Actor != nullptr ? Actor->GetInstigatorController<APlayerController>() : nullptr;
}
