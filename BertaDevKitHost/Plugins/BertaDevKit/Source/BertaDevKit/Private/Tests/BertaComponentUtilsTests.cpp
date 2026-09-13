#include "Component/BertaComponentUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsNullTest,
	"BertaDevKit.Component.Debug.Null",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsNullTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("A null component produces the minimal summary"),
		UBertaComponentUtils::GetAttachmentDebugSummary(nullptr),
		FString(TEXT("Component: None")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsStandaloneTest,
	"BertaDevKit.Component.Debug.Standalone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsStandaloneTest::RunTest(const FString& Parameters)
{
	USceneComponent* Component = NewObject<USceneComponent>(GetTransientPackage(), TEXT("StandaloneComponent"));
	Component->SetMobility(EComponentMobility::Movable);

	const FString Summary = UBertaComponentUtils::GetAttachmentDebugSummary(Component);
	TestTrue(TEXT("The component identity is present"), Summary.Contains(TEXT("Component: StandaloneComponent")));
	TestTrue(TEXT("The component class is present"), Summary.Contains(TEXT("Class: SceneComponent")));
	TestTrue(TEXT("A standalone component has no owner"), Summary.Contains(TEXT("Owner: None")));
	TestTrue(TEXT("A standalone component has no attachment parent"),
		Summary.Contains(TEXT("ParentComponent: None")) && Summary.Contains(TEXT("ParentOwner: None")));
	TestTrue(TEXT("A standalone component has no attachment socket"), Summary.Contains(TEXT("Socket: None")));
	TestTrue(TEXT("A standalone component is its own attachment root"),
		Summary.Contains(TEXT("RootComponent: StandaloneComponent")));
	TestTrue(TEXT("A standalone component without an owner has no root actor"), Summary.Contains(TEXT("RootActor: None")));
	TestTrue(TEXT("Both transform sections are present"),
		Summary.Contains(TEXT("Stored Relative Transform:")) && Summary.Contains(TEXT("World Transform:")));
	TestTrue(TEXT("All absolute flags are present"),
		Summary.Contains(TEXT("AbsoluteLocation: false"))
		&& Summary.Contains(TEXT("AbsoluteRotation: false"))
		&& Summary.Contains(TEXT("AbsoluteScale: false")));
	TestTrue(TEXT("Mobility is present"), Summary.Contains(TEXT("Mobility: Movable")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsAttachmentTest,
	"BertaDevKit.Component.Debug.Attachment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsAttachmentTest::RunTest(const FString& Parameters)
{
	AActor* ParentOwner = NewObject<AActor>(GetTransientPackage(), TEXT("ParentOwner"));
	AActor* ChildOwner = NewObject<AActor>(GetTransientPackage(), TEXT("ChildOwner"));
	USceneComponent* Parent = NewObject<USceneComponent>(ParentOwner, TEXT("ParentComponent"));
	USceneComponent* Child = NewObject<USceneComponent>(ChildOwner, TEXT("ChildComponent"));
	Child->SetupAttachment(Parent, TEXT("hand_r_socket"));

	const FString Summary = UBertaComponentUtils::GetAttachmentDebugSummary(Child);
	TestTrue(TEXT("The component owner is present"), Summary.Contains(TEXT("Owner: ChildOwner")));
	TestTrue(TEXT("The attachment parent is present"), Summary.Contains(TEXT("ParentComponent: ParentComponent")));
	TestTrue(TEXT("The attachment parent owner is present"), Summary.Contains(TEXT("ParentOwner: ParentOwner")));
	TestTrue(TEXT("The attachment socket is present"), Summary.Contains(TEXT("Socket: hand_r_socket")));
	TestTrue(TEXT("The native attachment root is present"), Summary.Contains(TEXT("RootComponent: ParentComponent")));
	TestTrue(TEXT("The native attachment root actor is present"), Summary.Contains(TEXT("RootActor: ParentOwner")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsTransformTest,
	"BertaDevKit.Component.Debug.Transform",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsTransformTest::RunTest(const FString& Parameters)
{
	USceneComponent* Component = NewObject<USceneComponent>();
	Component->SetRelativeLocation(FVector(1.25, -2.5, 3.75));
	Component->SetRelativeRotation(FRotator(10.0, 20.0, 30.0));
	Component->SetRelativeScale3D(FVector(1.5, 2.0, 0.5));
	Component->SetUsingAbsoluteLocation(true);
	Component->SetUsingAbsoluteRotation(false);
	Component->SetUsingAbsoluteScale(true);

	const FString Summary = UBertaComponentUtils::GetAttachmentDebugSummary(Component);
	TestTrue(TEXT("Stored relative location is formatted explicitly"),
		Summary.Contains(TEXT("Stored Relative Transform:\nLocation: X=1.250 Y=-2.500 Z=3.750")));
	TestTrue(TEXT("Stored relative rotation is formatted explicitly"),
		Summary.Contains(TEXT("Rotation: P=10.000 Y=20.000 R=30.000")));
	TestTrue(TEXT("Stored relative scale is formatted explicitly"),
		Summary.Contains(TEXT("Scale: X=1.500 Y=2.000 Z=0.500")));
	TestTrue(TEXT("Absolute location is present"), Summary.Contains(TEXT("AbsoluteLocation: true")));
	TestTrue(TEXT("Absolute rotation is present"), Summary.Contains(TEXT("AbsoluteRotation: false")));
	TestTrue(TEXT("Absolute scale is present"), Summary.Contains(TEXT("AbsoluteScale: true")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsMobilityTest,
	"BertaDevKit.Component.Debug.Mobility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsMobilityTest::RunTest(const FString& Parameters)
{
	USceneComponent* Component = NewObject<USceneComponent>();

	Component->SetMobility(EComponentMobility::Static);
	TestTrue(TEXT("Static mobility has a stable label"),
		UBertaComponentUtils::GetAttachmentDebugSummary(Component).Contains(TEXT("Mobility: Static")));

	Component->SetMobility(EComponentMobility::Stationary);
	TestTrue(TEXT("Stationary mobility has a stable label"),
		UBertaComponentUtils::GetAttachmentDebugSummary(Component).Contains(TEXT("Mobility: Stationary")));

	Component->SetMobility(EComponentMobility::Movable);
	TestTrue(TEXT("Movable mobility has a stable label"),
		UBertaComponentUtils::GetAttachmentDebugSummary(Component).Contains(TEXT("Mobility: Movable")));
	return true;
}

#endif
