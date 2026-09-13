// BertaComponentUtils.cpp
#include "Component/BertaComponentUtils.h"

#include "Components/SceneComponent.h"
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
		TEXT("Owner: %s\n\n")
		TEXT("Attachment:\n")
		TEXT("ParentComponent: %s\n")
		TEXT("ParentOwner: %s\n")
		TEXT("Socket: %s\n")
		TEXT("RootComponent: %s\n")
		TEXT("RootActor: %s\n\n")
		TEXT("Stored Relative Transform:\n")
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
