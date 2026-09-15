#include "DelegateInspector/BertaDelegateInspector.h"

#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "Log/BertaDevKitEditorLog.h"
#include "UObject/Class.h"
#include "UObject/FieldIterator.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/Object.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealType.h"

namespace BertaDelegateInspector
{
	TArray<FName> AssignFunctionOccurrences(int32 OccurrenceCount, TArray<FName> MatchingFunctions)
	{
		TArray<FName> Result;
		if (OccurrenceCount <= 0) { return Result; }
		MatchingFunctions.Sort(FNameLexicalLess());
		Result.Reserve(OccurrenceCount);
		if (MatchingFunctions.Num() > OccurrenceCount)
		{
			Result.AddDefaulted(OccurrenceCount);
			return Result;
		}
		Result.Append(MatchingFunctions);
		while (Result.Num() < OccurrenceCount)
		{
			Result.Add(MatchingFunctions.Num() == 1 ? MatchingFunctions[0] : NAME_None);
		}
		return Result;
	}

	static void CopyObject(const UObject* Object, FString& Name, FString& ClassName, FString& Path)
	{
		Name = Object->GetName();
		ClassName = Object->GetClass()->GetName();
		Path = Object->GetPathName();
	}

	static const TArray<FName>& CandidateNames(UClass* ListenerClass, TMap<UClass*, TArray<FName>>& Cache)
	{
		if (const TArray<FName>* Existing = Cache.Find(ListenerClass)) { return *Existing; }
		TArray<FName>& Names = Cache.Add(ListenerClass);
		TSet<FName> Seen;
		for (TFieldIterator<UFunction> It(ListenerClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			if (!Seen.Contains(It->GetFName()))
			{
				Seen.Add(It->GetFName());
				Names.Add(It->GetFName());
			}
		}
		Names.Sort(FNameLexicalLess());
		return Names;
	}

	static void InspectDelegate(const UObject* SourceObject, const FMulticastDelegateProperty* Property,
		int32 ArrayIndex, TMap<UClass*, TArray<FName>>& FunctionCache, FDelegate& OutDelegate)
	{
		OutDelegate.Name = Property->GetName();
		if (Property->ArrayDim > 1)
		{
			OutDelegate.Name += FString::Printf(TEXT("[%d]"), ArrayIndex);
		}
		OutDelegate.SignatureName = Property->SignatureFunction
			? Property->SignatureFunction->GetName() : TEXT("Signature unavailable");
		OutDelegate.Kind = Property->IsA<FMulticastSparseDelegateProperty>()
			? TEXT("Sparse dynamic multicast delegate") : TEXT("Dynamic multicast delegate");

		// ContainerPtrToValuePtr reads the instance's stored property bytes; it does not invoke a setter/getter.
		const void* Value = Property->ContainerPtrToValuePtr<void>(SourceObject, ArrayIndex);
		const FMulticastScriptDelegate* Multicast = Property->GetMulticastDelegate(Value);
		if (!Multicast) { return; } // Unbound sparse properties have no invocation-list allocation.

		// UE 5.8 exposes listener objects but not the individual script invocation entries.
		// GetAllObjects and Contains are read-only; neither compacts the source list.
		const TArray<UObject*> Objects = Multicast->GetAllObjects();
		TMap<UObject*, int32> Occurrences;
		for (UObject* Object : Objects)
		{
			if (IsValid(Object)) { ++Occurrences.FindOrAdd(Object); }
		}
		for (const TPair<UObject*, int32>& Entry : Occurrences)
		{
			UObject* Listener = Entry.Key;
			TArray<FName> MatchingFunctions;
			for (FName FunctionName : CandidateNames(Listener->GetClass(), FunctionCache))
			{
				const UFunction* EffectiveFunction = Listener->FindFunction(FunctionName);
				if (Property->SignatureFunction && EffectiveFunction
					&& Property->SignatureFunction->IsSignatureCompatibleWith(EffectiveFunction)
					&& Multicast->Contains(Listener, FunctionName))
				{
					MatchingFunctions.Add(FunctionName);
				}
			}
			for (FName FunctionName : AssignFunctionOccurrences(Entry.Value, MoveTemp(MatchingFunctions)))
			{
				FBinding& Binding = OutDelegate.Bindings.AddDefaulted_GetRef();
				CopyObject(Listener, Binding.Name, Binding.ClassName, Binding.Path);
				Binding.LiveObject = Listener;
				Binding.FunctionName = FunctionName;
				Binding.bFunctionResolved = FunctionName != NAME_None;
			}
		}
	}

	static void InspectSource(UObject* SourceObject, ESourceKind Kind,
		TMap<UClass*, TArray<FName>>& FunctionCache, FSnapshot& Snapshot)
	{
		FSource& Source = Snapshot.Sources.AddDefaulted_GetRef();
		CopyObject(SourceObject, Source.Name, Source.ClassName, Source.Path);
		Source.LiveObject = SourceObject;
		Source.Kind = Kind;
		for (TFieldIterator<FMulticastDelegateProperty> It(SourceObject->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			const FMulticastDelegateProperty* Property = *It;
			for (int32 Index = 0; Index < Property->ArrayDim; ++Index)
			{
				FDelegate& Delegate = Source.Delegates.AddDefaulted_GetRef();
				InspectDelegate(SourceObject, Property, Index, FunctionCache, Delegate);
			}
		}
	}

	bool Inspect(AActor* Target, FSnapshot& OutSnapshot, FString& OutError)
	{
		OutSnapshot = {};
		OutError.Reset();
		if (!IsInGameThread())
		{
			OutError = TEXT("Delegate inspection must run on the game thread.");
			UE_LOG(LogBertaDevKitEditor, Error, TEXT("[DelegateInspector] Called off the game thread."));
			return false;
		}
		if (!IsValid(Target) || Target->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject)
			|| !Target->GetWorld() || Target->GetWorld()->WorldType != EWorldType::PIE)
		{
			OutError = TEXT("The captured target is not a valid PIE Actor.");
			return false;
		}
		const double StartSeconds = FPlatformTime::Seconds();
		CopyObject(Target, OutSnapshot.TargetName, OutSnapshot.TargetClass, OutSnapshot.TargetPath);
		TMap<UClass*, TArray<FName>> FunctionCache;
		InspectSource(Target, ESourceKind::Actor, FunctionCache, OutSnapshot);
		TInlineComponentArray<UActorComponent*> Components(Target);
		for (UActorComponent* Component : Components)
		{
			if (IsValid(Component) && Component->GetOwner() == Target
				&& !Component->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
			{
				InspectSource(Component, ESourceKind::Component, FunctionCache, OutSnapshot);
			}
		}
		SortAndCount(OutSnapshot);
		OutSnapshot.DurationMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
		return true;
	}
}
