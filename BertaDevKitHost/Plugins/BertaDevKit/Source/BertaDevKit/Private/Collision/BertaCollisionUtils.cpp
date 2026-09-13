// BertaCollisionUtils.cpp
#include "Collision/BertaCollisionUtils.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"

namespace BertaCollisionUtilsPrivate
{
	const TCHAR* FormatBool(const bool bValue)
	{
		return bValue ? TEXT("true") : TEXT("false");
	}

	const TCHAR* FormatCollisionEnabled(const ECollisionEnabled::Type CollisionEnabled)
	{
		switch (CollisionEnabled)
		{
		case ECollisionEnabled::NoCollision: return TEXT("NoCollision");
		case ECollisionEnabled::QueryOnly: return TEXT("QueryOnly");
		case ECollisionEnabled::PhysicsOnly: return TEXT("PhysicsOnly");
		case ECollisionEnabled::QueryAndPhysics: return TEXT("QueryAndPhysics");
		case ECollisionEnabled::ProbeOnly: return TEXT("ProbeOnly");
		case ECollisionEnabled::QueryAndProbe: return TEXT("QueryAndProbe");
		default: return TEXT("Unknown");
		}
	}

	const TCHAR* FormatCollisionResponse(const ECollisionResponse Response)
	{
		switch (Response)
		{
		case ECR_Ignore: return TEXT("Ignore");
		case ECR_Overlap: return TEXT("Overlap");
		case ECR_Block: return TEXT("Block");
		default: return TEXT("Unknown");
		}
	}

	FString FormatCollisionChannel(const ECollisionChannel Channel)
	{
		const UCollisionProfile* CollisionProfile = UCollisionProfile::Get();
		if (CollisionProfile != nullptr)
		{
			const FName ChannelName = CollisionProfile->ReturnChannelNameFromContainerIndex(static_cast<int32>(Channel));
			if (!ChannelName.IsNone())
			{
				return ChannelName.ToString();
			}
		}

		return TEXT("Unknown");
	}

	ECollisionResponse ResolveInteraction(const ECollisionResponse AToB, const ECollisionResponse BToA)
	{
		if (AToB == ECR_Ignore || BToA == ECR_Ignore)
		{
			return ECR_Ignore;
		}

		if (AToB == ECR_Overlap || BToA == ECR_Overlap)
		{
			return ECR_Overlap;
		}

		return AToB == ECR_Block && BToA == ECR_Block ? ECR_Block : ECR_MAX;
	}

	FString BuildComponentSummary(
		const TCHAR* Label,
		const TCHAR* OtherLabel,
		const UPrimitiveComponent* Component,
		const UPrimitiveComponent* OtherComponent)
	{
		if (Component == nullptr)
		{
			return FString::Printf(TEXT("%s: None"), Label);
		}

		const AActor* Owner = Component->GetOwner();
		const FString OwnerLabel = Owner != nullptr ? Owner->GetName() : TEXT("Owner: None");
		const FName ProfileName = Component->GetCollisionProfileName();
		FString Summary = FString::Printf(
			TEXT("%s: %s [%s]\n")
			TEXT("Class: %s\n")
			TEXT("Profile: %s\n")
			TEXT("CollisionEnabled: %s\n")
			TEXT("ObjectType: %s\n"),
			Label,
			*Component->GetName(),
			*OwnerLabel,
			*Component->GetClass()->GetName(),
			ProfileName.IsNone() ? TEXT("None") : *ProfileName.ToString(),
			FormatCollisionEnabled(Component->GetCollisionEnabled()),
			*FormatCollisionChannel(Component->GetCollisionObjectType()));

		if (OtherComponent != nullptr)
		{
			const ECollisionChannel OtherObjectType = OtherComponent->GetCollisionObjectType();
			Summary += FString::Printf(
				TEXT("ResponseTo%s(%s): %s\n"),
				OtherLabel,
				*FormatCollisionChannel(OtherObjectType),
				FormatCollisionResponse(Component->GetCollisionResponseToChannel(OtherObjectType)));
		}

		Summary += FString::Printf(TEXT("GenerateOverlapEvents: %s"), FormatBool(Component->GetGenerateOverlapEvents()));
		return Summary;
	}

	void AppendNotes(FString& Summary, const TArray<FString>& Notes)
	{
		if (Notes.IsEmpty())
		{
			return;
		}

		Summary += TEXT("\n\nNotes:");
		for (const FString& Note : Notes)
		{
			Summary += FString::Printf(TEXT("\n%s"), *Note);
		}
	}
}

FString UBertaCollisionUtils::GetCollisionPairDebugSummary(
	const UPrimitiveComponent* A,
	const UPrimitiveComponent* B)
{
	FString Summary = BertaCollisionUtilsPrivate::BuildComponentSummary(TEXT("A"), TEXT("B"), A, B);
	Summary += TEXT("\n\n");
	Summary += BertaCollisionUtilsPrivate::BuildComponentSummary(TEXT("B"), TEXT("A"), B, A);

	if (A == nullptr || B == nullptr)
	{
		Summary += TEXT("\n\nPair:\nUnavailable");
		TArray<FString> Notes;
		if (A == nullptr && B != nullptr)
		{
			Notes.Add(TEXT("A is None."));
		}
		if (B == nullptr && A != nullptr)
		{
			Notes.Add(TEXT("B is None."));
		}
		BertaCollisionUtilsPrivate::AppendNotes(Summary, Notes);
		return Summary;
	}

	const ECollisionChannel AObjectType = A->GetCollisionObjectType();
	const ECollisionChannel BObjectType = B->GetCollisionObjectType();
	const ECollisionResponse AToB = A->GetCollisionResponseToChannel(BObjectType);
	const ECollisionResponse BToA = B->GetCollisionResponseToChannel(AObjectType);
	const ECollisionResponse ResolvedInteraction = BertaCollisionUtilsPrivate::ResolveInteraction(AToB, BToA);
	Summary += FString::Printf(
		TEXT("\n\nPair:\nResolvedInteraction: %s"),
		BertaCollisionUtilsPrivate::FormatCollisionResponse(ResolvedInteraction));

	TArray<FString> Notes;
	if (ResolvedInteraction == ECR_Ignore)
	{
		Notes.Add(TEXT("Pair resolves to Ignore because at least one side ignores the other's object type."));
		if (AToB == ECR_Ignore)
		{
			Notes.Add(FString::Printf(
				TEXT("A ignores B's %s channel."),
				*BertaCollisionUtilsPrivate::FormatCollisionChannel(BObjectType)));
		}
		if (BToA == ECR_Ignore)
		{
			Notes.Add(FString::Printf(
				TEXT("B ignores A's %s channel."),
				*BertaCollisionUtilsPrivate::FormatCollisionChannel(AObjectType)));
		}
	}

	const bool bAGeneratesOverlapEvents = A->GetGenerateOverlapEvents();
	const bool bBGeneratesOverlapEvents = B->GetGenerateOverlapEvents();
	if (!bAGeneratesOverlapEvents)
	{
		Notes.Add(TEXT("A does not generate overlap events."));
	}
	if (!bBGeneratesOverlapEvents)
	{
		Notes.Add(TEXT("B does not generate overlap events."));
	}
	if (!bAGeneratesOverlapEvents || !bBGeneratesOverlapEvents)
	{
		Notes.Add(TEXT("Both components must enable GenerateOverlapEvents for overlap notifications."));
	}

	const ECollisionEnabled::Type ACollisionEnabled = A->GetCollisionEnabled();
	const ECollisionEnabled::Type BCollisionEnabled = B->GetCollisionEnabled();
	const bool bAQueryEnabled = CollisionEnabledHasQuery(ACollisionEnabled);
	const bool bBQueryEnabled = CollisionEnabledHasQuery(BCollisionEnabled);
	if (!bAQueryEnabled)
	{
		Notes.Add(FString::Printf(
			TEXT("A uses %s and does not participate in query collision."),
			BertaCollisionUtilsPrivate::FormatCollisionEnabled(ACollisionEnabled)));
	}
	if (!bBQueryEnabled)
	{
		Notes.Add(FString::Printf(
			TEXT("B uses %s and does not participate in query collision."),
			BertaCollisionUtilsPrivate::FormatCollisionEnabled(BCollisionEnabled)));
	}

	if (ResolvedInteraction == ECR_Overlap
		&& bAQueryEnabled
		&& bBQueryEnabled
		&& bAGeneratesOverlapEvents
		&& bBGeneratesOverlapEvents)
	{
		Notes.Add(TEXT("No obvious overlap-configuration blocker found."));
		Notes.Add(TEXT("Geometry, movement, registration, lifecycle, and timing can still determine whether an overlap event occurs."));
	}
	else if (ResolvedInteraction == ECR_Block)
	{
		Notes.Add(TEXT("Pair is configured to Block. Hit-event notification settings are outside this V1 diagnostic."));
	}

	BertaCollisionUtilsPrivate::AppendNotes(Summary, Notes);
	return Summary;
}
