#include "Actor/BertaActorUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AIController.h"
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
