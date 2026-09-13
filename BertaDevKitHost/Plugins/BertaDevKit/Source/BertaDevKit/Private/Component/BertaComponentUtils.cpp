// BertaComponentUtils.cpp
#include "Component/BertaComponentUtils.h"

#include "ComponentInstanceDataCache.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"

namespace BertaComponentUtilsPrivate
{
	const TCHAR* FormatBool(const bool bValue)
	{
		return bValue ? TEXT("true") : TEXT("false");
	}

	const TCHAR* FormatMobility(const EComponentMobility::Type Mobility)
	{
		switch (Mobility)
		{
		case EComponentMobility::Static: return TEXT("Static");
		case EComponentMobility::Stationary: return TEXT("Stationary");
		case EComponentMobility::Movable: return TEXT("Movable");
		default: return TEXT("Unknown");
		}
	}

	const TCHAR* FormatTickGroup(const ETickingGroup TickGroup)
	{
		switch (TickGroup)
		{
		case TG_PrePhysics: return TEXT("PrePhysics");
		case TG_StartPhysics: return TEXT("StartPhysics");
		case TG_DuringPhysics: return TEXT("DuringPhysics");
		case TG_EndPhysics: return TEXT("EndPhysics");
		case TG_PostPhysics: return TEXT("PostPhysics");
		case TG_PostUpdateWork: return TEXT("PostUpdateWork");
		case TG_LastDemotable: return TEXT("LastDemotable");
		case TG_NewlySpawned: return TEXT("NewlySpawned");
		default: return TEXT("Unknown");
		}
	}

	const TCHAR* FormatCreationMethod(const EComponentCreationMethod CreationMethod)
	{
		switch (CreationMethod)
		{
		case EComponentCreationMethod::Native: return TEXT("Native");
		case EComponentCreationMethod::SimpleConstructionScript: return TEXT("SimpleConstructionScript");
		case EComponentCreationMethod::UserConstructionScript: return TEXT("UserConstructionScript");
		case EComponentCreationMethod::Instance: return TEXT("Instance");
		default: return TEXT("Unknown");
		}
	}

	FString FormatObjectName(const UObject* Object)
	{
		return Object != nullptr ? Object->GetName() : TEXT("None");
	}

	FString FormatVector(const FVector& Vector)
	{
		return FString::Printf(TEXT("X=%.3f Y=%.3f Z=%.3f"), Vector.X, Vector.Y, Vector.Z);
	}

	FString FormatRotator(const FRotator& Rotator)
	{
		return FString::Printf(TEXT("P=%.3f Y=%.3f R=%.3f"), Rotator.Pitch, Rotator.Yaw, Rotator.Roll);
	}
}

FString UBertaComponentUtils::GetAttachmentDebugSummary(const USceneComponent* Component)
{
	if (Component == nullptr)
	{
		return TEXT("Component: None");
	}

	const USceneComponent* AttachParent = Component->GetAttachParent();
	const USceneComponent* AttachmentRoot = Component->GetAttachmentRoot();
	const FName AttachSocketName = Component->GetAttachSocketName();
	const FString SocketName = AttachSocketName.IsNone() ? TEXT("None") : AttachSocketName.ToString();

	const FString RelativeLocation = BertaComponentUtilsPrivate::FormatVector(Component->GetRelativeLocation());
	const FString RelativeRotation = BertaComponentUtilsPrivate::FormatRotator(Component->GetRelativeRotation());
	const FString RelativeScale = BertaComponentUtilsPrivate::FormatVector(Component->GetRelativeScale3D());
	const FString WorldLocation = BertaComponentUtilsPrivate::FormatVector(Component->GetComponentLocation());
	const FString WorldRotation = BertaComponentUtilsPrivate::FormatRotator(Component->GetComponentRotation());
	const FString WorldScale = BertaComponentUtilsPrivate::FormatVector(Component->GetComponentScale());

	return FString::Printf(
		TEXT("Component: %s\n")
		TEXT("Class: %s\n")
		TEXT("Owner: %s\n")
		TEXT("Registered: %s\n\n")
		TEXT("Attachment:\n")
		TEXT("AttachParentComponent: %s\n")
		TEXT("AttachParentActor: %s\n")
		TEXT("AttachSocket: %s\n")
		TEXT("AttachmentRootComponent: %s\n")
		TEXT("AttachmentRootActor: %s\n\n")
		TEXT("Stored Transform Properties:\n")
		TEXT("Location: %s\n")
		TEXT("Rotation: %s\n")
		TEXT("Scale: %s\n\n")
		TEXT("World Transform:\n")
		TEXT("Location: %s\n")
		TEXT("Rotation: %s\n")
		TEXT("Scale: %s\n\n")
		TEXT("Transform Inheritance:\n")
		TEXT("AbsoluteLocation: %s\n")
		TEXT("AbsoluteRotation: %s\n")
		TEXT("AbsoluteScale: %s\n\n")
		TEXT("Mobility: %s"),
		*Component->GetName(),
		*Component->GetClass()->GetName(),
		*BertaComponentUtilsPrivate::FormatObjectName(Component->GetOwner()),
		BertaComponentUtilsPrivate::FormatBool(Component->IsRegistered()),
		*BertaComponentUtilsPrivate::FormatObjectName(AttachParent),
		*BertaComponentUtilsPrivate::FormatObjectName(Component->GetAttachParentActor()),
		*SocketName,
		*BertaComponentUtilsPrivate::FormatObjectName(AttachmentRoot),
		*BertaComponentUtilsPrivate::FormatObjectName(Component->GetAttachmentRootActor()),
		*RelativeLocation,
		*RelativeRotation,
		*RelativeScale,
		*WorldLocation,
		*WorldRotation,
		*WorldScale,
		BertaComponentUtilsPrivate::FormatBool(Component->IsUsingAbsoluteLocation()),
		BertaComponentUtilsPrivate::FormatBool(Component->IsUsingAbsoluteRotation()),
		BertaComponentUtilsPrivate::FormatBool(Component->IsUsingAbsoluteScale()),
		BertaComponentUtilsPrivate::FormatMobility(Component->GetMobility()));
}

FString UBertaComponentUtils::GetLifecycleDebugSummary(const UActorComponent* Component)
{
	if (Component == nullptr)
	{
		return TEXT("Component: None");
	}

	const FActorComponentTickFunction& TickFunction = Component->PrimaryComponentTick;
	return FString::Printf(
		TEXT("Component: %s\n")
		TEXT("Class: %s\n")
		TEXT("Owner: %s\n\n")
		TEXT("Lifecycle:\n")
		TEXT("Registered: %s\n")
		TEXT("Initialized: %s\n")
		TEXT("BegunPlay: %s\n")
		TEXT("Active: %s\n")
		TEXT("BeingDestroyed: %s\n\n")
		TEXT("Tick:\n")
		TEXT("CanEverTick: %s\n")
		TEXT("TickFunctionRegistered: %s\n")
		TEXT("TickEnabled: %s\n")
		TEXT("StartWithTickEnabled: %s\n")
		TEXT("TickGroup: %s\n")
		TEXT("TickInterval: %.3f\n")
		TEXT("TickEvenWhenPaused: %s\n")
		TEXT("AllowTickOnDedicatedServer: %s\n\n")
		TEXT("Configuration:\n")
		TEXT("AutoRegister: %s\n")
		TEXT("WantsInitializeComponent: %s\n")
		TEXT("AutoActivate: %s\n")
		TEXT("Replicated: %s\n")
		TEXT("CreationMethod: %s"),
		*Component->GetName(),
		*Component->GetClass()->GetName(),
		*BertaComponentUtilsPrivate::FormatObjectName(Component->GetOwner()),
		BertaComponentUtilsPrivate::FormatBool(Component->IsRegistered()),
		BertaComponentUtilsPrivate::FormatBool(Component->HasBeenInitialized()),
		BertaComponentUtilsPrivate::FormatBool(Component->HasBegunPlay()),
		BertaComponentUtilsPrivate::FormatBool(Component->IsActive()),
		BertaComponentUtilsPrivate::FormatBool(Component->IsBeingDestroyed()),
		BertaComponentUtilsPrivate::FormatBool(TickFunction.bCanEverTick),
		BertaComponentUtilsPrivate::FormatBool(TickFunction.IsTickFunctionRegistered()),
		BertaComponentUtilsPrivate::FormatBool(Component->IsComponentTickEnabled()),
		BertaComponentUtilsPrivate::FormatBool(TickFunction.bStartWithTickEnabled),
		BertaComponentUtilsPrivate::FormatTickGroup(TickFunction.TickGroup),
		Component->GetComponentTickInterval(),
		BertaComponentUtilsPrivate::FormatBool(TickFunction.bTickEvenWhenPaused),
		BertaComponentUtilsPrivate::FormatBool(TickFunction.bAllowTickOnDedicatedServer),
		BertaComponentUtilsPrivate::FormatBool(Component->bAutoRegister),
		BertaComponentUtilsPrivate::FormatBool(Component->bWantsInitializeComponent),
		BertaComponentUtilsPrivate::FormatBool(Component->bAutoActivate),
		BertaComponentUtilsPrivate::FormatBool(Component->GetIsReplicated()),
		BertaComponentUtilsPrivate::FormatCreationMethod(Component->CreationMethod));
}
