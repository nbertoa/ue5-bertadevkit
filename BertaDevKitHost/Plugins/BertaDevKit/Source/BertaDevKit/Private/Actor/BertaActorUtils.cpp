// BertaActorUtils.cpp
#include "Actor/BertaActorUtils.h"

#include "AIController.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

namespace
{
	const TCHAR* FormatBool(const bool bValue)
	{
		return bValue ? TEXT("true") : TEXT("false");
	}

	const TCHAR* FormatNetMode(const ENetMode NetMode)
	{
		switch (NetMode)
		{
		case NM_Standalone: return TEXT("Standalone");
		case NM_DedicatedServer: return TEXT("DedicatedServer");
		case NM_ListenServer: return TEXT("ListenServer");
		case NM_Client: return TEXT("Client");
		default: return TEXT("Unknown");
		}
	}

	const TCHAR* FormatNetRole(const ENetRole NetRole)
	{
		switch (NetRole)
		{
		case ROLE_None: return TEXT("None");
		case ROLE_SimulatedProxy: return TEXT("SimulatedProxy");
		case ROLE_AutonomousProxy: return TEXT("AutonomousProxy");
		case ROLE_Authority: return TEXT("Authority");
		default: return TEXT("Unknown");
		}
	}

	FString GetActorName(const AActor* Actor)
	{
		return Actor != nullptr ? Actor->GetName() : TEXT("None");
	}
}

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

FString UBertaActorUtils::GetNetworkDebugSummary(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return TEXT("Actor: None");
	}

	const FString OwnerName = GetActorName(Actor->GetOwner());
	const FString NetOwnerName = GetActorName(Actor->GetNetOwner());
	const FString InstigatorName = GetActorName(Actor->GetInstigator());
	FString Summary = FString::Printf(
		TEXT("Actor: %s\n")
		TEXT("Class: %s\n\n")
		TEXT("NetMode: %s\n")
		TEXT("LocalRole: %s\n")
		TEXT("RemoteRole: %s\n")
		TEXT("Authority: %s\n")
		TEXT("Replicates: %s\n")
		TEXT("ReplicateMovement: %s\n\n")
		TEXT("Owner: %s\n")
		TEXT("NetOwner: %s\n")
		TEXT("HasNetOwner: %s\n")
		TEXT("HasLocalNetOwner: %s\n")
		TEXT("Instigator: %s"),
		*Actor->GetName(),
		*Actor->GetClass()->GetName(),
		FormatNetMode(Actor->GetNetMode()),
		FormatNetRole(Actor->GetLocalRole()),
		FormatNetRole(Actor->GetRemoteRole()),
		FormatBool(Actor->HasAuthority()),
		FormatBool(Actor->GetIsReplicated()),
		FormatBool(Actor->IsReplicatingMovement()),
		*OwnerName,
		*NetOwnerName,
		FormatBool(Actor->HasNetOwner()),
		FormatBool(Actor->HasLocalNetOwner()),
		*InstigatorName);

	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		const FString ControllerName = GetActorName(Pawn->GetController());
		Summary += FString::Printf(
			TEXT("\n\nPawn.Controller: %s\n")
			TEXT("Pawn.PlayerControlled: %s\n")
			TEXT("Pawn.LocallyControlled: %s"),
			*ControllerName,
			FormatBool(Pawn->IsPlayerControlled()),
			FormatBool(Pawn->IsLocallyControlled()));
	}

	if (const AController* Controller = Cast<AController>(Actor))
	{
		const FString ControlledPawnName = GetActorName(Controller->GetPawn());
		Summary += FString::Printf(
			TEXT("\n\nController.ControlledPawn: %s\n")
			TEXT("Controller.LocalController: %s\n")
			TEXT("Controller.PlayerController: %s"),
			*ControlledPawnName,
			FormatBool(Controller->IsLocalController()),
			FormatBool(Controller->IsPlayerController()));
	}

	return Summary;
}
