// BertaActorUtils.cpp
#include "Actor/BertaActorUtils.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

APlayerController* UBertaActorUtils::GetPlayerControllerFromPawn(const APawn* Pawn)
{
	return Pawn != nullptr ? Pawn->GetController<APlayerController>() : nullptr;
}

UAnimInstance* UBertaActorUtils::GetAnimInstanceFromCharacter(const ACharacter* Character)
{
	const USkeletalMeshComponent* Mesh = Character != nullptr ? Character->GetMesh() : nullptr;
	return Mesh != nullptr ? Mesh->GetAnimInstance() : nullptr;
}

APlayerController* UBertaActorUtils::GetInstigatorPlayerController(const AActor* Actor)
{
	return Actor != nullptr ? Actor->GetInstigatorController<APlayerController>() : nullptr;
}
