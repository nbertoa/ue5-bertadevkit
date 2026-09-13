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
	TestTrue(TEXT("The actual registration state is present"),
		Summary.Contains(FString::Printf(
			TEXT("Registered: %s"),
			Component->IsRegistered() ? TEXT("true") : TEXT("false"))));
	TestTrue(TEXT("A standalone component has no attachment parent"),
		Summary.Contains(TEXT("AttachParentComponent: None"))
		&& Summary.Contains(TEXT("AttachParentActor: None")));
	TestTrue(TEXT("A standalone component has no attachment socket"), Summary.Contains(TEXT("AttachSocket: None")));
	TestTrue(TEXT("A standalone component is its own attachment root"),
		Summary.Contains(TEXT("AttachmentRootComponent: StandaloneComponent")));
	TestTrue(TEXT("A standalone component without an owner has no root actor"),
		Summary.Contains(TEXT("AttachmentRootActor: None")));
	TestTrue(TEXT("Both transform sections are present"),
		Summary.Contains(TEXT("Stored Transform Properties:")) && Summary.Contains(TEXT("World Transform:")));
	TestTrue(TEXT("All absolute flags are present"),
		Summary.Contains(TEXT("AbsoluteLocation: false"))
		&& Summary.Contains(TEXT("AbsoluteRotation: false"))
		&& Summary.Contains(TEXT("AbsoluteScale: false")));
	TestTrue(TEXT("Mobility is present"), Summary.Contains(TEXT("Mobility: Movable")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsOwnerTest,
	"BertaDevKit.Component.Debug.Owner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsOwnerTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage(), TEXT("ComponentOwner"));
	USceneComponent* Component = NewObject<USceneComponent>(Owner, TEXT("OwnedComponent"));

	const FString Summary = UBertaComponentUtils::GetAttachmentDebugSummary(Component);
	TestTrue(TEXT("The direct owner is present"), Summary.Contains(TEXT("Owner: ComponentOwner")));
	TestTrue(TEXT("An unattached owned component is its own attachment root"),
		Summary.Contains(TEXT("AttachmentRootComponent: OwnedComponent")));
	TestTrue(TEXT("The native attachment root actor is the component owner"),
		Summary.Contains(TEXT("AttachmentRootActor: ComponentOwner")));
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
	TestTrue(TEXT("The attachment parent is present"),
		Summary.Contains(TEXT("AttachParentComponent: ParentComponent")));
	TestTrue(TEXT("The attachment parent actor is present"),
		Summary.Contains(TEXT("AttachParentActor: ParentOwner")));
	TestTrue(TEXT("The attachment socket is present"), Summary.Contains(TEXT("AttachSocket: hand_r_socket")));
	TestTrue(TEXT("The native attachment root is present"),
		Summary.Contains(TEXT("AttachmentRootComponent: ParentComponent")));
	TestTrue(TEXT("The native attachment root actor is present"),
		Summary.Contains(TEXT("AttachmentRootActor: ParentOwner")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsAttachmentHierarchyTest,
	"BertaDevKit.Component.Debug.AttachmentHierarchy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsAttachmentHierarchyTest::RunTest(const FString& Parameters)
{
	AActor* ActorA = NewObject<AActor>(GetTransientPackage(), TEXT("ActorA"));
	AActor* ActorB = NewObject<AActor>(GetTransientPackage(), TEXT("ActorB"));
	AActor* ActorC = NewObject<AActor>(GetTransientPackage(), TEXT("ActorC"));
	USceneComponent* Root = NewObject<USceneComponent>(ActorA, TEXT("RootComponent"));
	USceneComponent* Middle = NewObject<USceneComponent>(ActorB, TEXT("MiddleComponent"));
	USceneComponent* Child = NewObject<USceneComponent>(ActorC, TEXT("ChildComponent"));
	Middle->SetupAttachment(Root);
	Child->SetupAttachment(Middle);

	const FString Summary = UBertaComponentUtils::GetAttachmentDebugSummary(Child);
	TestTrue(TEXT("The direct attachment parent differs from the hierarchy root"),
		Summary.Contains(TEXT("AttachParentComponent: MiddleComponent"))
		&& Summary.Contains(TEXT("AttachmentRootComponent: RootComponent")));
	TestTrue(TEXT("The direct parent actor differs from the hierarchy root actor"),
		Summary.Contains(TEXT("AttachParentActor: ActorB"))
		&& Summary.Contains(TEXT("AttachmentRootActor: ActorA")));
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
	TestTrue(TEXT("Stored location is formatted explicitly"),
		Summary.Contains(TEXT("Stored Transform Properties:\nLocation: X=1.250 Y=-2.500 Z=3.750")));
	TestTrue(TEXT("Stored rotation is formatted explicitly"),
		Summary.Contains(TEXT("Rotation: P=10.000 Y=20.000 R=30.000")));
	TestTrue(TEXT("Stored scale is formatted explicitly"),
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
