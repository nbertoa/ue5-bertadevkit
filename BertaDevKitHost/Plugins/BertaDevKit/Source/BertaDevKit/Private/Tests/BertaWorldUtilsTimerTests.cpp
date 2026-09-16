#include "World/BertaWorldUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	struct FScopedTimerWorld
	{
		FScopedTimerWorld()
		{
			const UWorld::InitializationValues Values = UWorld::InitializationValues()
				.CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false)
				.ShouldSimulatePhysics(false).EnableTraceCollision(false);
			World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr,
				true, ERHIFeatureLevel::Num, &Values);
		}

		~FScopedTimerWorld()
		{
			if (World)
			{
				World->DestroyWorld(false);
			}
		}

		FTimerHandle SeedHandle() const
		{
			FTimerHandle Handle;
			World->GetTimerManager().SetTimer(Handle, 60.0f, false);
			return Handle;
		}

		UWorld* World = nullptr;
	};

	FTimerDynamicDelegate MakeCallback(UObject* Outer)
	{
		// A reflected no-argument callback; these tests only schedule, never tick.
		UActorComponent* Receiver = NewObject<USceneComponent>(Outer);
		FTimerDynamicDelegate Callback;
		Callback.BindDynamic(Receiver, &UActorComponent::Deactivate);
		return Callback;
	}

	void TestFailureOutput(FAutomationTestBase& Test, UWorld* World,
		const FTimerHandle OutHandle, FTimerHandle PreviousHandle)
	{
		Test.TestFalse(TEXT("Failure invalidates the output handle"), OutHandle.IsValid());
		Test.TestTrue(TEXT("Failure preserves the previous timer"), World->GetTimerManager().TimerExists(PreviousHandle));
		World->GetTimerManager().ClearTimer(PreviousHandle);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaDelayedActionInvalidContextTest,
	"BertaDevKit.World.Timers.InvalidContextClearsHandle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaDelayedActionInvalidContextTest::RunTest(const FString& Parameters)
{
	FScopedTimerWorld Scope;
	if (!TestNotNull(TEXT("A lightweight world is available"), Scope.World))
	{
		return false;
	}

	const FTimerDynamicDelegate Callback = MakeCallback(Scope.World);
	const UObject* Contexts[] = { nullptr, NewObject<UObject>() };
	for (const UObject* Context : Contexts)
	{
		const FTimerHandle PreviousHandle = Scope.SeedHandle();
		FTimerHandle OutHandle = PreviousHandle;
		TestTrue(TEXT("The reused handle starts valid"), OutHandle.IsValid());
		AddExpectedError(Context ? TEXT("Could not retrieve UWorld") : TEXT("WorldContextObject is null or pending kill"),
			EAutomationExpectedErrorFlags::Contains, 1);
		TestFalse(TEXT("Invalid context fails"), UBertaWorldUtils::SetDelayedAction(Context, Callback, 1.0f, OutHandle));
		TestFailureOutput(*this, Scope.World, OutHandle, PreviousHandle);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaDelayedActionUnboundCallbackTest,
	"BertaDevKit.World.Timers.UnboundCallbackClearsHandle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaDelayedActionUnboundCallbackTest::RunTest(const FString& Parameters)
{
	FScopedTimerWorld Scope;
	if (!TestNotNull(TEXT("A lightweight world is available"), Scope.World))
	{
		return false;
	}

	const FTimerHandle PreviousHandle = Scope.SeedHandle();
	FTimerHandle OutHandle = PreviousHandle;
	TestTrue(TEXT("The reused handle starts valid"), OutHandle.IsValid());
	AddExpectedError(TEXT("Callback delegate is not bound"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Unbound callback fails"), UBertaWorldUtils::SetDelayedAction(Scope.World, {}, 1.0f, OutHandle));
	TestFailureOutput(*this, Scope.World, OutHandle, PreviousHandle);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaDelayedActionInvalidDelayTest,
	"BertaDevKit.World.Timers.InvalidDelayClearsHandle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaDelayedActionInvalidDelayTest::RunTest(const FString& Parameters)
{
	FScopedTimerWorld Scope;
	if (!TestNotNull(TEXT("A lightweight world is available"), Scope.World))
	{
		return false;
	}

	const FTimerDynamicDelegate Callback = MakeCallback(Scope.World);
	for (const float Delay : { 0.0f, -1.0f })
	{
		const FTimerHandle PreviousHandle = Scope.SeedHandle();
		FTimerHandle OutHandle = PreviousHandle;
		TestTrue(TEXT("The reused handle starts valid"), OutHandle.IsValid());
		AddExpectedError(TEXT("must be > 0. Timer not set"), EAutomationExpectedErrorFlags::Contains, 1);
		TestFalse(TEXT("Non-positive delay fails"), UBertaWorldUtils::SetDelayedAction(Scope.World, Callback, Delay, OutHandle));
		TestFailureOutput(*this, Scope.World, OutHandle, PreviousHandle);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaDelayedActionSuccessTest,
	"BertaDevKit.World.Timers.SuccessCreatesIndependentTimer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaDelayedActionSuccessTest::RunTest(const FString& Parameters)
{
	FScopedTimerWorld Scope;
	if (!TestNotNull(TEXT("A lightweight world is available"), Scope.World))
	{
		return false;
	}

	FTimerHandle PreviousHandle = Scope.SeedHandle();
	FTimerHandle OutHandle = PreviousHandle;
	TestTrue(TEXT("The reused handle starts valid"), OutHandle.IsValid());
	TestTrue(TEXT("A bound callback and positive delay succeed"),
		UBertaWorldUtils::SetDelayedAction(Scope.World, MakeCallback(Scope.World), 1.0f, OutHandle));
	TestTrue(TEXT("Success returns a valid handle"), OutHandle.IsValid());
	TestTrue(TEXT("The new timer exists without ticking"), Scope.World->GetTimerManager().TimerExists(OutHandle));
	TestTrue(TEXT("The output identifies a different timer"), OutHandle != PreviousHandle);
	TestTrue(TEXT("Success preserves the previous timer"), Scope.World->GetTimerManager().TimerExists(PreviousHandle));
	UBertaWorldUtils::CancelDelayedAction(Scope.World, OutHandle);
	TestFalse(TEXT("The new timer can be canceled"), OutHandle.IsValid());
	TestTrue(TEXT("Canceling the new timer preserves the old timer"), Scope.World->GetTimerManager().TimerExists(PreviousHandle));
	Scope.World->GetTimerManager().ClearTimer(PreviousHandle);
	return true;
}

#endif
