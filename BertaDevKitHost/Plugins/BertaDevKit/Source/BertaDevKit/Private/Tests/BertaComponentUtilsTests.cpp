#include "Component/BertaComponentUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "ComponentInstanceDataCache.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

namespace BertaComponentUtilsTestsPrivate
{
	bool ContainsBoolField(const FString& Summary, const TCHAR* Label, const bool bValue)
	{
		return Summary.Contains(FString::Printf(
			TEXT("%s: %s"),
			Label,
			bValue ? TEXT("true") : TEXT("false")));
	}
}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsLifecycleNullTest,
	"BertaDevKit.Component.Debug.Lifecycle.Null",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsLifecycleNullTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("A null component produces the minimal lifecycle summary"),
		UBertaComponentUtils::GetLifecycleDebugSummary(nullptr),
		FString(TEXT("Component: None")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsLifecycleDefaultTest,
	"BertaDevKit.Component.Debug.Lifecycle.Default",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsLifecycleDefaultTest::RunTest(const FString& Parameters)
{
	USceneComponent* Component = NewObject<USceneComponent>(GetTransientPackage(), TEXT("LifecycleComponent"));
	const FString Summary = UBertaComponentUtils::GetLifecycleDebugSummary(Component);

	TestTrue(TEXT("The lifecycle component identity is present"),
		Summary.Contains(TEXT("Component: LifecycleComponent"))
		&& Summary.Contains(TEXT("Class: SceneComponent"))
		&& Summary.Contains(TEXT("Owner: None")));
	TestTrue(TEXT("Registered reflects the component state"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(Summary, TEXT("Registered"), Component->IsRegistered()));
	TestTrue(TEXT("Initialized reflects the component state"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("Initialized"), Component->HasBeenInitialized()));
	TestTrue(TEXT("BegunPlay reflects the component state"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(Summary, TEXT("BegunPlay"), Component->HasBegunPlay()));
	TestTrue(TEXT("Active reflects the component state"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(Summary, TEXT("Active"), Component->IsActive()));
	TestTrue(TEXT("BeingDestroyed reflects the component state"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("BeingDestroyed"), Component->IsBeingDestroyed()));
	TestTrue(TEXT("CanEverTick reflects the tick function"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("CanEverTick"), Component->PrimaryComponentTick.bCanEverTick));
	TestTrue(TEXT("TickFunctionRegistered reflects the tick function"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary,
			TEXT("TickFunctionRegistered"),
			Component->PrimaryComponentTick.IsTickFunctionRegistered()));
	TestTrue(TEXT("TickEnabled reflects the component state"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("TickEnabled"), Component->IsComponentTickEnabled()));
	TestTrue(TEXT("StartWithTickEnabled reflects the tick function"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("StartWithTickEnabled"), Component->PrimaryComponentTick.bStartWithTickEnabled));
	TestTrue(TEXT("Tick interval reflects the component state"),
		Summary.Contains(FString::Printf(TEXT("TickInterval: %.3f"), Component->GetComponentTickInterval())));
	TestTrue(TEXT("TickEvenWhenPaused reflects the tick function"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("TickEvenWhenPaused"), Component->PrimaryComponentTick.bTickEvenWhenPaused));
	TestTrue(TEXT("AllowTickOnDedicatedServer reflects the tick function"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary,
			TEXT("AllowTickOnDedicatedServer"),
			Component->PrimaryComponentTick.bAllowTickOnDedicatedServer));
	TestTrue(TEXT("AutoRegister reflects component configuration"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("AutoRegister"), Component->bAutoRegister));
	TestTrue(TEXT("WantsInitializeComponent reflects component configuration"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("WantsInitializeComponent"), Component->bWantsInitializeComponent));
	TestTrue(TEXT("AutoActivate reflects component configuration"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("AutoActivate"), Component->bAutoActivate));
	TestTrue(TEXT("Replicated reflects component configuration"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			Summary, TEXT("Replicated"), Component->GetIsReplicated()));
	TestTrue(TEXT("Tick group and creation method are present"),
		Summary.Contains(TEXT("TickGroup:")) && Summary.Contains(TEXT("CreationMethod:")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsLifecycleOwnerTest,
	"BertaDevKit.Component.Debug.Lifecycle.Owner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsLifecycleOwnerTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>(GetTransientPackage(), TEXT("LifecycleOwner"));
	USceneComponent* Component = NewObject<USceneComponent>(Owner, TEXT("OwnedLifecycleComponent"));

	TestTrue(TEXT("The lifecycle summary includes the owner"),
		UBertaComponentUtils::GetLifecycleDebugSummary(Component).Contains(TEXT("Owner: LifecycleOwner")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsLifecycleTickTest,
	"BertaDevKit.Component.Debug.Lifecycle.Tick",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsLifecycleTickTest::RunTest(const FString& Parameters)
{
	USceneComponent* Component = NewObject<USceneComponent>();
	Component->PrimaryComponentTick.bCanEverTick = true;
	Component->PrimaryComponentTick.bStartWithTickEnabled = true;
	Component->PrimaryComponentTick.bTickEvenWhenPaused = true;
	Component->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	Component->SetComponentTickInterval(0.25f);

	Component->PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	const FString ConfiguredSummary = UBertaComponentUtils::GetLifecycleDebugSummary(Component);
	TestTrue(TEXT("Configured tick booleans are present"),
		ConfiguredSummary.Contains(TEXT("CanEverTick: true"))
		&& ConfiguredSummary.Contains(TEXT("StartWithTickEnabled: true"))
		&& ConfiguredSummary.Contains(TEXT("TickEvenWhenPaused: true"))
		&& ConfiguredSummary.Contains(TEXT("AllowTickOnDedicatedServer: false")));
	TestTrue(TEXT("Configured tick interval is present"), ConfiguredSummary.Contains(TEXT("TickInterval: 0.250")));
	TestTrue(TEXT("DuringPhysics uses a stable label"), ConfiguredSummary.Contains(TEXT("TickGroup: DuringPhysics")));

	Component->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	TestTrue(TEXT("PrePhysics uses a stable label"),
		UBertaComponentUtils::GetLifecycleDebugSummary(Component).Contains(TEXT("TickGroup: PrePhysics")));
	Component->PrimaryComponentTick.TickGroup = TG_PostPhysics;
	TestTrue(TEXT("PostPhysics uses a stable label"),
		UBertaComponentUtils::GetLifecycleDebugSummary(Component).Contains(TEXT("TickGroup: PostPhysics")));
	Component->PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
	TestTrue(TEXT("PostUpdateWork uses a stable label"),
		UBertaComponentUtils::GetLifecycleDebugSummary(Component).Contains(TEXT("TickGroup: PostUpdateWork")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaComponentUtilsLifecycleConfigurationTest,
	"BertaDevKit.Component.Debug.Lifecycle.Configuration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaComponentUtilsLifecycleConfigurationTest::RunTest(const FString& Parameters)
{
	USceneComponent* Component = NewObject<USceneComponent>();
	Component->bAutoRegister = false;
	Component->bWantsInitializeComponent = true;
	Component->bAutoActivate = false;

	const FString ConfigurationSummary = UBertaComponentUtils::GetLifecycleDebugSummary(Component);
	TestTrue(TEXT("Lifecycle configuration booleans are present"),
		ConfigurationSummary.Contains(TEXT("AutoRegister: false"))
		&& ConfigurationSummary.Contains(TEXT("WantsInitializeComponent: true"))
		&& ConfigurationSummary.Contains(TEXT("AutoActivate: false")));
	TestTrue(TEXT("Replicated reports the actual component value"),
		BertaComponentUtilsTestsPrivate::ContainsBoolField(
			ConfigurationSummary, TEXT("Replicated"), Component->GetIsReplicated()));

	Component->CreationMethod = EComponentCreationMethod::Native;
	TestTrue(TEXT("Native creation uses a stable label"),
		UBertaComponentUtils::GetLifecycleDebugSummary(Component).Contains(TEXT("CreationMethod: Native")));
	Component->CreationMethod = EComponentCreationMethod::SimpleConstructionScript;
	TestTrue(TEXT("SimpleConstructionScript creation uses a stable label"),
		UBertaComponentUtils::GetLifecycleDebugSummary(Component).Contains(
			TEXT("CreationMethod: SimpleConstructionScript")));
	Component->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	TestTrue(TEXT("UserConstructionScript creation uses a stable label"),
		UBertaComponentUtils::GetLifecycleDebugSummary(Component).Contains(
			TEXT("CreationMethod: UserConstructionScript")));
	Component->CreationMethod = EComponentCreationMethod::Instance;
	TestTrue(TEXT("Instance creation uses a stable label"),
		UBertaComponentUtils::GetLifecycleDebugSummary(Component).Contains(TEXT("CreationMethod: Instance")));
	return true;
}

#endif
