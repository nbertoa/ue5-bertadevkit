#include "ObjectGraph/BertaObjectGraphAnalyzer.h"

#include "Components/ActorComponent.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "Selection.h"
#include "UObject/GCObjectInfo.h"
#include "UObject/ReferenceChainSearch.h"

namespace
{
	// Actor typed-element data and ordinary Details views use weak references,
	// but other selection observers may affect a live GC search. Neutralize the
	// visible selection conservatively, with only weak saved handles, and restore
	// it even on failure. This creates no undo transaction.
	class FNeutralSelection
	{
	public:
		FNeutralSelection()
		{
			USelection* Actors = GEditor->GetSelectedActors();
			USelection* Components = GEditor->GetSelectedComponents();
			for (int32 Index = 0; Index < Actors->Num(); ++Index)
			{
				if (AActor* Actor = Cast<AActor>(Actors->GetSelectedObject(Index))) { SavedActors.Add(Actor); }
			}
			for (int32 Index = 0; Index < Components->Num(); ++Index)
			{
				if (UActorComponent* Component = Cast<UActorComponent>(Components->GetSelectedObject(Index))) { SavedComponents.Add(Component); }
			}
			GEditor->SelectNone(true, false, false);
			Components->DeselectAll();
			bCleared = Actors->CountSelections(AActor::StaticClass()) == 0 && Components->CountSelections(UActorComponent::StaticClass()) == 0;
		}

		~FNeutralSelection()
		{
			GEditor->GetSelectedActors()->BeginBatchSelectOperation();
			GEditor->GetSelectedComponents()->BeginBatchSelectOperation();
			for (const TWeakObjectPtr<AActor>& WeakActor : SavedActors)
			{
				if (AActor* Actor = WeakActor.Get()) { GEditor->SelectActor(Actor, true, false, true); }
			}
			for (const TWeakObjectPtr<UActorComponent>& WeakComponent : SavedComponents)
			{
				if (UActorComponent* Component = WeakComponent.Get()) { GEditor->SelectComponent(Component, true, false, true); }
			}
			GEditor->GetSelectedComponents()->EndBatchSelectOperation();
			GEditor->GetSelectedActors()->EndBatchSelectOperation();
		}

		bool WasCleared() const { return bCleared; }

	private:
		TArray<TWeakObjectPtr<AActor>> SavedActors;
		TArray<TWeakObjectPtr<UActorComponent>> SavedComponents;
		bool bCleared = false;
	};

	FString DescribeReference(const FReferenceChainSearch::FNodeReferenceInfo* Info)
	{
		if (!Info) { return TEXT("Unknown GC reference"); }
		switch (Info->Type)
		{
		case FReferenceChainSearch::EReferenceType::Property:
		case FReferenceChainSearch::EReferenceType::AddReferencedObjects:
			return Info->ToString();
		case FReferenceChainSearch::EReferenceType::OuterChain:
			return TEXT("Outer Chain");
		default:
			return TEXT("Unknown GC reference");
		}
	}
}

namespace BertaObjectGraph
{
	bool Analyze(TWeakObjectPtr<UObject> Target, FSnapshot& OutSnapshot, FString& OutError)
	{
		check(IsInGameThread());
		if (!GEditor)
		{
			OutError = TEXT("Editor selection is unavailable.");
			return false;
		}
		FNeutralSelection NeutralSelection;
		if (!NeutralSelection.WasCleared())
		{
			OutError = TEXT("Editor selection could not be cleared; analysis was cancelled.");
			return false;
		}
		UObject* LiveTarget = Target.Get();
		if (!LiveTarget)
		{
			OutError = TEXT("Captured actor expired before analysis.");
			return false;
		}
		const double Start = FPlatformTime::Seconds();
		TArray<FChain> Chains;
		int32 NativeCount = 0;
		bool bDirectRoot = false;
		{
			// Shortest is per root. Do not use ExternalOnly: UE 5.8 filters
			// complete chains out in that mode. Do not enable PrintResults.
			FReferenceChainSearch Search(LiveTarget, EReferenceChainSearchMode::Shortest);
			NativeCount = Search.GetReferenceChains().Num();
			for (const FReferenceChainSearch::FReferenceChain* NativeChain : Search.GetReferenceChains())
			{
				FChain& Chain = Chains.AddDefaulted_GetRef();
				for (int32 NativeIndex = NativeChain->Num() - 1; NativeIndex >= 0; --NativeIndex)
				{
					FGCObjectInfo* Info = NativeChain->GetNode(NativeIndex)->ObjectInfo;
					FStep& Step = Chain.AddDefaulted_GetRef();
					Step.Object.Name = Info->GetName().ToString();
					Step.Object.ClassName = Info->GetClassName();
					Step.Object.Path = Info->GetPathName();
					Step.Object.RootFlags = FReferenceChainSearch::GetObjectFlags(*Info);
					Step.Object.LiveObject = Info->TryResolveObject();
					if (NativeIndex > 0)
					{
						Step.ReasonToNext = DescribeReference(NativeChain->GetReferenceInfo(NativeIndex));
					}
				}
			}
		}
		// A directly rooted target can have no incoming graph edge, so the
		// engine's path search emits no chain. Surface that root explicitly.
		const bool bTargetIsNativeRoot = LiveTarget->HasAnyInternalFlags(EInternalObjectFlags_RootFlags);
		const bool bTargetHasRefCount = LiveTarget->GetRefCount() > 0;
		const bool bTargetHasKeepFlags = GARBAGE_COLLECTION_KEEPFLAGS != RF_NoFlags
			&& LiveTarget->HasAnyFlags(GARBAGE_COLLECTION_KEEPFLAGS);
		if (NativeCount == 0 && (bTargetIsNativeRoot || bTargetHasRefCount || bTargetHasKeepFlags))
		{
			FChain& Chain = Chains.AddDefaulted_GetRef();
			FStep& Step = Chain.AddDefaulted_GetRef();
			Step.Object.Name = LiveTarget->GetName();
			Step.Object.ClassName = LiveTarget->GetClass()->GetName();
			Step.Object.Path = LiveTarget->GetPathName();
			Step.Object.RootFlags = LiveTarget->HasAnyInternalFlags(EInternalObjectFlags::RootSet)
				? TEXT("(root)") : bTargetHasRefCount ? TEXT("(refcounted)") : TEXT("(native or keep-flags root)");
			Step.Object.LiveObject = LiveTarget;
			bDirectRoot = true;
		}
		OutSnapshot = BuildSnapshot(MoveTemp(Chains), NativeCount);
		OutSnapshot.bDirectRoot = bDirectRoot;
		OutSnapshot.DurationMs = (FPlatformTime::Seconds() - Start) * 1000.0;
		return true;
	}
}
