#include "World/BertaWorldUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "UObject/Object.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaWorldUtilsDebugInvalidContextTest,
	"BertaDevKit.World.Debug.InvalidContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaWorldUtilsDebugInvalidContextTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("A null context produces the minimal summary"),
		UBertaWorldUtils::GetWorldDebugSummary(nullptr),
		FString(TEXT("World: None")));

	const UObject* ObjectWithoutWorld = NewObject<UObject>();
	TestEqual(TEXT("An object without a world does not use a global fallback"),
		UBertaWorldUtils::GetWorldDebugSummary(ObjectWithoutWorld),
		FString(TEXT("World: None")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaWorldUtilsDebugGameWorldTest,
	"BertaDevKit.World.Debug.GameWorld",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaWorldUtilsDebugGameWorldTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("BertaWorldDebugGame"));
	if (!TestNotNull(TEXT("A lightweight game world can be created"), World))
	{
		return false;
	}

	const FString Summary = UBertaWorldUtils::GetWorldDebugSummary(World);
	TestTrue(TEXT("The world identity is present"), Summary.Contains(TEXT("World: BertaWorldDebugGame")));
	TestTrue(TEXT("The game world type uses a stable label"), Summary.Contains(TEXT("WorldType: Game")));
	TestTrue(TEXT("A non-PIE package has no PIE instance"), Summary.Contains(TEXT("PIEInstance: None")));
	TestTrue(TEXT("A lightweight game world is standalone"), Summary.Contains(TEXT("NetMode: Standalone")));
	TestTrue(TEXT("A lightweight world has no game instance"), Summary.Contains(TEXT("GameInstance: None")));
	TestTrue(TEXT("A lightweight world has no game mode"), Summary.Contains(TEXT("GameMode: None")));
	TestTrue(TEXT("A lightweight world has no game state"), Summary.Contains(TEXT("GameState: None")));
	TestTrue(TEXT("A lightweight world has no player controllers"), Summary.Contains(TEXT("PlayerControllers: 0")));

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaWorldUtilsDebugWorldTypeTest,
	"BertaDevKit.World.Debug.WorldType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaWorldUtilsDebugWorldTypeTest::RunTest(const FString& Parameters)
{
	UWorld* PIEWorld = UWorld::CreateWorld(EWorldType::PIE, false, TEXT("BertaWorldDebugPIE"));
	if (!TestNotNull(TEXT("A lightweight PIE world can be created"), PIEWorld))
	{
		return false;
	}

	PIEWorld->GetPackage()->SetPIEInstanceID(7);
	const FString PIESummary = UBertaWorldUtils::GetWorldDebugSummary(PIEWorld);
	TestTrue(TEXT("PIE uses a stable world type label"), PIESummary.Contains(TEXT("WorldType: PIE")));
	TestTrue(TEXT("PIE instance comes from the world package"), PIESummary.Contains(TEXT("PIEInstance: 7")));
	PIEWorld->DestroyWorld(false);

	UWorld* PreviewWorld = UWorld::CreateWorld(EWorldType::EditorPreview, false, TEXT("BertaWorldDebugPreview"));
	if (!TestNotNull(TEXT("A lightweight editor preview world can be created"), PreviewWorld))
	{
		return false;
	}

	TestTrue(TEXT("EditorPreview uses a stable world type label"),
		UBertaWorldUtils::GetWorldDebugSummary(PreviewWorld).Contains(TEXT("WorldType: EditorPreview")));
	PreviewWorld->DestroyWorld(false);
	return true;
}

#endif
