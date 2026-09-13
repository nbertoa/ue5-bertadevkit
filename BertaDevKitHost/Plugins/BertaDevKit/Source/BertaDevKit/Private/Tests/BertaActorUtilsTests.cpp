#include "Actor/BertaActorUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AIController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsPlayerControllerFromPawnTest,
	"BertaDevKit.Actor.Traversal.PlayerControllerFromPawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsPlayerControllerFromPawnTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null pawn has no player controller"), UBertaActorUtils::GetPlayerControllerFromPawn(nullptr));

	APawn* Pawn = NewObject<APawn>();
	TestNull(TEXT("An unpossessed pawn has no player controller"),
	         UBertaActorUtils::GetPlayerControllerFromPawn(Pawn));

	Pawn->SetController(NewObject<AAIController>());
	TestNull(TEXT("An AI-controlled pawn has no player controller"),
	         UBertaActorUtils::GetPlayerControllerFromPawn(Pawn));

	APlayerController* PlayerController = NewObject<APlayerController>();
	Pawn->SetController(PlayerController);
	TestEqual(TEXT("A player-controlled pawn returns its player controller"),
	          UBertaActorUtils::GetPlayerControllerFromPawn(Pawn), PlayerController);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsLocalPlayerControllerFromPawnTest,
	"BertaDevKit.Actor.Traversal.LocalPlayerControllerFromPawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsLocalPlayerControllerFromPawnTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null pawn has no local player controller"),
	         UBertaActorUtils::GetLocalPlayerControllerFromPawn(nullptr));

	APawn* Pawn = NewObject<APawn>();
	TestNull(TEXT("An unpossessed pawn has no local player controller"),
	         UBertaActorUtils::GetLocalPlayerControllerFromPawn(Pawn));

	Pawn->SetController(NewObject<AAIController>());
	TestNull(TEXT("An AI-controlled pawn has no local player controller"),
	         UBertaActorUtils::GetLocalPlayerControllerFromPawn(Pawn));

	APlayerController* PlayerController = NewObject<APlayerController>();
	Pawn->SetController(PlayerController);
	TestNull(TEXT("A pawn with a non-local player controller returns null"),
	         UBertaActorUtils::GetLocalPlayerControllerFromPawn(Pawn));

	PlayerController->SetAsLocalPlayerController();
	TestEqual(TEXT("A pawn with a local player controller returns that controller"),
	          UBertaActorUtils::GetLocalPlayerControllerFromPawn(Pawn), PlayerController);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsPlayerCameraManagerFromPawnTest,
	"BertaDevKit.Actor.Traversal.PlayerCameraManagerFromPawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsPlayerCameraManagerFromPawnTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null pawn has no player camera manager"),
	         UBertaActorUtils::GetPlayerCameraManagerFromPawn(nullptr));

	APawn* Pawn = NewObject<APawn>();
	TestNull(TEXT("An unpossessed pawn has no player camera manager"),
	         UBertaActorUtils::GetPlayerCameraManagerFromPawn(Pawn));

	Pawn->SetController(NewObject<AAIController>());
	TestNull(TEXT("An AI-controlled pawn has no player camera manager"),
	         UBertaActorUtils::GetPlayerCameraManagerFromPawn(Pawn));

	APlayerController* PlayerController = NewObject<APlayerController>();
	Pawn->SetController(PlayerController);
	TestNull(TEXT("A player controller without a camera manager returns null"),
	         UBertaActorUtils::GetPlayerCameraManagerFromPawn(Pawn));

	APlayerCameraManager* CameraManager = NewObject<APlayerCameraManager>();
	PlayerController->PlayerCameraManager = CameraManager;
	TestEqual(TEXT("A pawn returns its player controller's camera manager"),
	          UBertaActorUtils::GetPlayerCameraManagerFromPawn(Pawn), CameraManager);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsBlackboardComponentFromPawnTest,
	"BertaDevKit.Actor.Traversal.BlackboardComponentFromPawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsBlackboardComponentFromPawnTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null pawn has no blackboard component"),
	         UBertaActorUtils::GetBlackboardComponentFromPawn(nullptr));

	APawn* Pawn = NewObject<APawn>();
	TestNull(TEXT("An unpossessed pawn has no blackboard component"),
	         UBertaActorUtils::GetBlackboardComponentFromPawn(Pawn));

	Pawn->SetController(NewObject<APlayerController>());
	TestNull(TEXT("A player-controlled pawn has no AI blackboard component"),
	         UBertaActorUtils::GetBlackboardComponentFromPawn(Pawn));

	Pawn->SetController(NewObject<AAIController>());
	TestNull(TEXT("An AI controller without a blackboard component returns null"),
	         UBertaActorUtils::GetBlackboardComponentFromPawn(Pawn));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsAnimInstanceFromCharacterTest,
	"BertaDevKit.Actor.Traversal.AnimInstanceFromCharacter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsAnimInstanceFromCharacterTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null character has no animation instance"),
	         UBertaActorUtils::GetAnimInstanceFromCharacter(nullptr));
	TestNull(TEXT("A character without an initialized animation instance returns null"),
	         UBertaActorUtils::GetAnimInstanceFromCharacter(NewObject<ACharacter>()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsOwnerPawnTest,
	"BertaDevKit.Actor.Traversal.OwnerPawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsOwnerPawnTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null component has no owner pawn"), UBertaActorUtils::GetOwnerPawn(nullptr));
	TestNull(TEXT("A component without an actor owner has no owner pawn"),
	         UBertaActorUtils::GetOwnerPawn(NewObject<USceneComponent>()));
	TestNull(TEXT("A component owned by a non-pawn actor has no owner pawn"),
	         UBertaActorUtils::GetOwnerPawn(NewObject<USceneComponent>(NewObject<AActor>())));

	APawn* Pawn = NewObject<APawn>();
	TestEqual(TEXT("A component owned by a pawn returns that pawn"),
	          UBertaActorUtils::GetOwnerPawn(NewObject<USceneComponent>(Pawn)), Pawn);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsOwnerCharacterTest,
	"BertaDevKit.Actor.Traversal.OwnerCharacter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsOwnerCharacterTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null component has no owner character"), UBertaActorUtils::GetOwnerCharacter(nullptr));
	TestNull(TEXT("A component without an actor owner has no owner character"),
	         UBertaActorUtils::GetOwnerCharacter(NewObject<USceneComponent>()));
	TestNull(TEXT("A component owned by a non-character pawn has no owner character"),
	         UBertaActorUtils::GetOwnerCharacter(NewObject<USceneComponent>(NewObject<APawn>())));

	ACharacter* Character = NewObject<ACharacter>();
	TestEqual(TEXT("A component owned by a character returns that character"),
	          UBertaActorUtils::GetOwnerCharacter(NewObject<USceneComponent>(Character)), Character);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsInstigatorCharacterTest,
	"BertaDevKit.Actor.Traversal.InstigatorCharacter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsInstigatorCharacterTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null actor has no instigator character"),
	         UBertaActorUtils::GetInstigatorCharacter(nullptr));

	AActor* Actor = NewObject<AActor>();
	TestNull(TEXT("An actor without an instigator has no instigator character"),
	         UBertaActorUtils::GetInstigatorCharacter(Actor));

	Actor->SetInstigator(NewObject<APawn>());
	TestNull(TEXT("A non-character instigator returns null"),
	         UBertaActorUtils::GetInstigatorCharacter(Actor));

	ACharacter* Character = NewObject<ACharacter>();
	Actor->SetInstigator(Character);
	TestEqual(TEXT("A character instigator is returned"),
	          UBertaActorUtils::GetInstigatorCharacter(Actor), Character);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsInstigatorPlayerControllerTest,
	"BertaDevKit.Actor.Traversal.InstigatorPlayerController",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsInstigatorPlayerControllerTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("A null actor has no instigator player controller"),
	         UBertaActorUtils::GetInstigatorPlayerController(nullptr));

	AActor* Actor = NewObject<AActor>();
	TestNull(TEXT("An actor without an instigator has no player controller"),
	         UBertaActorUtils::GetInstigatorPlayerController(Actor));

	APawn* Instigator = NewObject<APawn>();
	Instigator->SetController(NewObject<AAIController>());
	Actor->SetInstigator(Instigator);
	TestNull(TEXT("An AI-controlled instigator has no player controller"),
	         UBertaActorUtils::GetInstigatorPlayerController(Actor));

	APlayerController* PlayerController = NewObject<APlayerController>();
	Instigator->SetController(PlayerController);
	TestEqual(TEXT("An actor with a player-controlled instigator returns its player controller"),
	          UBertaActorUtils::GetInstigatorPlayerController(Actor), PlayerController);
	return true;
}

#endif
