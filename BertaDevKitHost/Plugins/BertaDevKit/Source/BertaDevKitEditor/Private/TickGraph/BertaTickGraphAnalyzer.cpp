#include "TickGraph/BertaTickGraphAnalyzer.h"

#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "CoreGlobals.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "Log/BertaDevKitEditorLog.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

namespace BertaTickGraph
{
	namespace
	{
		struct FSeed
		{
			const FTickFunction* Tick = nullptr;
			UObject* Owner = nullptr;
			ERole Role = ERole::TargetActorPrimary;
		};

		struct FResolvedPrerequisite
		{
			const FTickFunction* Tick = nullptr;
			UObject* Owner = nullptr;
			FString SortKey;
			int32 NativeIndex = 0;
		};

		FString GroupName(TEnumAsByte<ETickingGroup> Group)
		{
			const UEnum* Enum = StaticEnum<ETickingGroup>();
			return Enum ? Enum->GetNameStringByValue(Group.GetValue()) : FString::FromInt(Group.GetValue());
		}

		struct FTraversal
		{
			AActor* Target;
			TSet<const UObject*> SeedOwners;
			TMap<const FTickFunction*, int32> Ids; // Only exists during this synchronous Game Thread capture.
			FBuilder Builder;

			explicit FTraversal(AActor* InTarget) : Target(InTarget) {}

			int32 FindOrAdd(const FTickFunction* Tick, UObject* Owner, ERole Role)
			{
			if (const int32* Existing = Ids.Find(Tick)) { return *Existing; }
			EKind Kind = EKind::Custom;
			if (const AActor* Actor = Cast<AActor>(Owner); Actor && Tick == &Actor->PrimaryActorTick) { Kind = EKind::ActorPrimary; }
			else if (const UActorComponent* Component = Cast<UActorComponent>(Owner); Component && Tick == &Component->PrimaryComponentTick)
			{
				Kind = EKind::ComponentPrimary;
			}
			FNode Node;
			Node.Name = Owner->GetName();
			Node.OwnerName = Owner->GetName();
			Node.OwnerClass = Owner->GetClass()->GetName();
			Node.OwnerPath = Owner->GetPathName();
			Node.LiveOwner = Owner;
			Node.Role = Role;
			Node.Kind = Kind;
			// Arbitrary custom overrides of the non-const diagnostic virtuals have no read-only contract.
			Node.TickGroup = GroupName(Tick->TickGroup);
			Node.EndTickGroup = GroupName(Tick->EndTickGroup);
			Node.Interval = Tick->TickInterval;
			Node.bCanEverTick = Tick->bCanEverTick;
			Node.bEnabled = Tick->IsTickFunctionEnabled();
			Node.bRegistered = Tick->IsTickFunctionRegistered();
			Node.bHighPriority = Tick->bHighPriority;
			Node.bAnyThread = Tick->bRunOnAnyThread;
			Node.bPaused = Tick->bTickEvenWhenPaused;
			Node.bStartEnabled = Tick->bStartWithTickEnabled;
			Node.bDedicatedServer = Tick->bAllowTickOnDedicatedServer;
			Node.bDispatchManually = Tick->bDispatchManually;
			Node.DirectPrerequisites = Tick->GetPrerequisites().Num();
			const int32 Id = Builder.AddNode(MoveTemp(Node));
			if (Id != INDEX_NONE)
			{
				if (Kind == EKind::Custom)
				{
					Builder.Data().Nodes[Id].Name += FString::Printf(TEXT(" [Custom Tick %d]"), Id);
				}
				Ids.Add(Tick, Id);
				if (!SeedOwners.Contains(Owner)) { ++Builder.Data().ExternalCount; }
			}
			return Id;
			}

			void Visit(const FTickFunction* Tick, int32 Depth)
			{
			const int32 Current = Ids.FindChecked(Tick);
			if (!Builder.Enter(Current, Depth)) { return; }
			const TArray<FTickPrerequisite>& Native = Tick->GetPrerequisites();
			if (Depth >= MaxDepth && !Native.IsEmpty())
			{
				Builder.MarkTruncated();
				Builder.Leave(Current);
				return;
			}
			TArray<FResolvedPrerequisite> Resolved;
			Resolved.Reserve(Native.Num());
			for (int32 Index = 0; Index < Native.Num(); ++Index)
			{
				const FTickPrerequisite& Record = Native[Index];
				const FTickFunction* PrerequisiteTick = Record.Get();
				UObject* PrerequisiteOwner = Record.PrerequisiteObject.Get();
				if (!PrerequisiteTick || !IsValid(PrerequisiteOwner))
				{
					++Builder.Data().InvalidPrerequisiteCount;
					continue;
				}
				FResolvedPrerequisite& Entry = Resolved.AddDefaulted_GetRef();
				Entry.Tick = PrerequisiteTick;
				Entry.Owner = PrerequisiteOwner;
				Entry.SortKey = PrerequisiteOwner->GetPathName();
				Entry.NativeIndex = Index;
			}
			Resolved.Sort([](const FResolvedPrerequisite& A, const FResolvedPrerequisite& B)
			{
				return A.SortKey == B.SortKey ? A.NativeIndex < B.NativeIndex : A.SortKey < B.SortKey;
			});
			for (const FResolvedPrerequisite& Entry : Resolved)
			{
				ERole Role = SeedOwners.Contains(Entry.Owner) ? ERole::CustomPrerequisite : ERole::ExternalPrerequisite;
				if (Entry.Owner == Target && Entry.Tick == &Target->PrimaryActorTick) { Role = ERole::TargetActorPrimary; }
				else if (const UActorComponent* Component = Cast<UActorComponent>(Entry.Owner);
					Component && SeedOwners.Contains(Component) && Entry.Tick == &Component->PrimaryComponentTick)
				{
					Role = ERole::TargetComponentPrimary;
				}
				const int32 Prerequisite = FindOrAdd(Entry.Tick, Entry.Owner, Role);
				if (Prerequisite == INDEX_NONE) { continue; }
				if (!Builder.AddEdge(Prerequisite, Current)) { break; }
				Visit(Entry.Tick, Depth + 1);
			}
			Builder.Leave(Current);
			}
		};
	}

	bool Capture(AActor* Target, FSnapshot& OutSnapshot, FString& Error)
	{
		if (!IsInGameThread() || !IsValid(Target) || Target->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			Error = TEXT("The captured target is no longer valid.");
			UE_LOG(LogBertaDevKitEditor, Warning, TEXT("[BertaTickGraph] Capture rejected an invalid target or thread."));
			return false;
		}
		if (!Target->GetWorld() || Target->GetWorld()->WorldType != EWorldType::PIE)
		{
			Error = TEXT("The selected Actor does not belong to a PIE world.");
			return false;
		}
		const double Start = FPlatformTime::Seconds();
		FTraversal Traversal(Target);
		TArray<FSeed> Seeds;
		Seeds.Add({&Target->PrimaryActorTick, Target, ERole::TargetActorPrimary});
		TInlineComponentArray<UActorComponent*> Components(Target, false);
		Components.Sort([](const UActorComponent& A, const UActorComponent& B) { return A.GetPathName() < B.GetPathName(); });
		for (UActorComponent* Component : Components)
		{
			if (IsValid(Component) && Component->GetOwner() == Target && !Component->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
			{
				Seeds.Add({&Component->PrimaryComponentTick, Component, ERole::TargetComponentPrimary});
			}
		}
		Traversal.SeedOwners.Add(Target);
		for (const FSeed& Seed : Seeds) { Traversal.SeedOwners.Add(Seed.Owner); }
		for (const FSeed& Seed : Seeds)
		{
			if (Traversal.FindOrAdd(Seed.Tick, Seed.Owner, Seed.Role) != INDEX_NONE) { ++Traversal.Builder.Data().SeedCount; }
		}
		for (const FSeed& Seed : Seeds)
		{
			if (Traversal.Ids.Contains(Seed.Tick)) { Traversal.Visit(Seed.Tick, 0); }
		}
		OutSnapshot = Traversal.Builder.Finish();
		OutSnapshot.TargetName = Target->GetName();
		OutSnapshot.TargetClass = Target->GetClass()->GetName();
		OutSnapshot.TargetPath = Target->GetPathName();
		OutSnapshot.DurationMs = (FPlatformTime::Seconds() - Start) * 1000.0;
		return true;
	}
}
