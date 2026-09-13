#include "Actor/BertaActorUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AIController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaActorUtilsNetworkDebugSummaryTest,
	"BertaDevKit.Actor.Debug.NetworkSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaActorUtilsNetworkDebugSummaryTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("A null actor has the minimal summary"),
	          UBertaActorUtils::GetNetworkDebugSummary(nullptr), FString(TEXT("Actor: None")));

	AActor* Actor = NewObject<AActor>();
	const FString ActorSummary = UBertaActorUtils::GetNetworkDebugSummary(Actor);
	TestTrue(TEXT("A plain actor includes its name"),
	         ActorSummary.Contains(FString::Printf(TEXT("Actor: %s"), *Actor->GetName())));
	TestTrue(TEXT("A plain actor includes its class"),
	         ActorSummary.Contains(FString::Printf(TEXT("Class: %s"), *Actor->GetClass()->GetName())));
	TestTrue(TEXT("A transient actor reports standalone net mode"), ActorSummary.Contains(TEXT("NetMode: Standalone")));
	TestTrue(TEXT("A plain actor includes local role"), ActorSummary.Contains(TEXT("LocalRole: Authority")));
	TestTrue(TEXT("A plain actor includes remote role"), ActorSummary.Contains(TEXT("RemoteRole: None")));
	TestTrue(TEXT("A plain actor includes authority"), ActorSummary.Contains(TEXT("Authority: true")));
	TestTrue(TEXT("A plain actor includes replication state"), ActorSummary.Contains(TEXT("Replicates: false")));
	TestTrue(TEXT("A plain actor includes movement replication state"), ActorSummary.Contains(TEXT("ReplicateMovement: false")));
	TestTrue(TEXT("A plain actor renders a null owner as None"), ActorSummary.Contains(TEXT("Owner: None")));
	TestTrue(TEXT("A plain actor renders a null net owner as None"), ActorSummary.Contains(TEXT("NetOwner: None")));
	TestTrue(TEXT("A plain actor includes net ownership state"), ActorSummary.Contains(TEXT("HasNetOwner: false")));
	TestTrue(TEXT("A plain actor includes local net ownership state"), ActorSummary.Contains(TEXT("HasLocalNetOwner: false")));
	TestTrue(TEXT("A plain actor renders a null instigator as None"), ActorSummary.Contains(TEXT("Instigator: None")));
	TestFalse(TEXT("A plain actor omits the Pawn section"), ActorSummary.Contains(TEXT("Pawn.")));
	TestFalse(TEXT("A plain actor omits the Controller section"), ActorSummary.Contains(TEXT("Controller.")));

	APawn* UnpossessedPawn = NewObject<APawn>();
	const FString UnpossessedPawnSummary = UBertaActorUtils::GetNetworkDebugSummary(UnpossessedPawn);
	TestTrue(TEXT("An unpossessed pawn renders a null controller as None"),
	         UnpossessedPawnSummary.Contains(TEXT("Pawn.Controller: None")));
	TestTrue(TEXT("An unpossessed pawn reports the actual player-controlled state"),
	         UnpossessedPawnSummary.Contains(FString::Printf(TEXT("Pawn.PlayerControlled: %s"), UnpossessedPawn->IsPlayerControlled() ? TEXT("true") : TEXT("false"))));
	TestTrue(TEXT("An unpossessed pawn reports the actual locally-controlled state"),
	         UnpossessedPawnSummary.Contains(FString::Printf(TEXT("Pawn.LocallyControlled: %s"), UnpossessedPawn->IsLocallyControlled() ? TEXT("true") : TEXT("false"))));
	TestFalse(TEXT("A pawn omits the Controller section"), UnpossessedPawnSummary.Contains(TEXT("Controller.")));

	APawn* AIPawn = NewObject<APawn>();
	AAIController* AIController = NewObject<AAIController>();
	AIPawn->SetController(AIController);
	const FString AIPawnSummary = UBertaActorUtils::GetNetworkDebugSummary(AIPawn);
	TestTrue(TEXT("An AI pawn includes its controller name"),
	         AIPawnSummary.Contains(FString::Printf(TEXT("Pawn.Controller: %s"), *AIController->GetName())));
	TestTrue(TEXT("An AI pawn reports the actual player-controlled state"),
	         AIPawnSummary.Contains(FString::Printf(TEXT("Pawn.PlayerControlled: %s"), AIPawn->IsPlayerControlled() ? TEXT("true") : TEXT("false"))));
	TestTrue(TEXT("An AI pawn reports the actual locally-controlled state"),
	         AIPawnSummary.Contains(FString::Printf(TEXT("Pawn.LocallyControlled: %s"), AIPawn->IsLocallyControlled() ? TEXT("true") : TEXT("false"))));

	APawn* PlayerPawn = NewObject<APawn>();
	APlayerController* PlayerController = NewObject<APlayerController>();
	APlayerState* PlayerState = NewObject<APlayerState>();
	PlayerState->SetIsABot(false);
	PlayerPawn->SetController(PlayerController);
	PlayerPawn->SetPlayerState(PlayerState);
	const FString PlayerPawnSummary = UBertaActorUtils::GetNetworkDebugSummary(PlayerPawn);
	TestTrue(TEXT("A player pawn includes its controller name"),
	         PlayerPawnSummary.Contains(FString::Printf(TEXT("Pawn.Controller: %s"), *PlayerController->GetName())));
	TestTrue(TEXT("A player pawn reports the actual player-controlled state"),
	         PlayerPawnSummary.Contains(FString::Printf(TEXT("Pawn.PlayerControlled: %s"), PlayerPawn->IsPlayerControlled() ? TEXT("true") : TEXT("false"))));
	TestTrue(TEXT("A player pawn reports the actual locally-controlled state"),
	         PlayerPawnSummary.Contains(FString::Printf(TEXT("Pawn.LocallyControlled: %s"), PlayerPawn->IsLocallyControlled() ? TEXT("true") : TEXT("false"))));

	PlayerController->SetAsLocalPlayerController();
	const FString LocalPlayerPawnSummary = UBertaActorUtils::GetNetworkDebugSummary(PlayerPawn);
	TestTrue(TEXT("A pawn with a configured local player controller reports local control"),
	         LocalPlayerPawnSummary.Contains(TEXT("Pawn.LocallyControlled: true")));

	AAIController* UnpossessedAIController = NewObject<AAIController>();
	const FString AIControllerSummary = UBertaActorUtils::GetNetworkDebugSummary(UnpossessedAIController);
	TestTrue(TEXT("An unpossessed controller renders a null pawn as None"),
	         AIControllerSummary.Contains(TEXT("Controller.ControlledPawn: None")));
	TestTrue(TEXT("An AI controller reports the actual local-controller state"),
	         AIControllerSummary.Contains(FString::Printf(TEXT("Controller.LocalController: %s"), UnpossessedAIController->IsLocalController() ? TEXT("true") : TEXT("false"))));
	TestTrue(TEXT("An AI controller is not reported as a player controller"),
	         AIControllerSummary.Contains(TEXT("Controller.PlayerController: false")));
	TestFalse(TEXT("A controller omits the Pawn section"), AIControllerSummary.Contains(TEXT("Pawn.")));

	APlayerController* LocalPlayerController = NewObject<APlayerController>();
	LocalPlayerController->SetAsLocalPlayerController();
	const FString LocalPlayerControllerSummary = UBertaActorUtils::GetNetworkDebugSummary(LocalPlayerController);
	TestTrue(TEXT("A local player controller reports local controller state"),
	         LocalPlayerControllerSummary.Contains(TEXT("Controller.LocalController: true")));
	TestTrue(TEXT("A player controller uses the engine type query"),
	         LocalPlayerControllerSummary.Contains(TEXT("Controller.PlayerController: true")));
	return true;
}

#endif
