#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"

class UObject;

namespace BertaDelegateInspector
{
	enum class ESourceKind : uint8 { Actor, Component };

	struct FBinding
	{
		FString Name;
		FString ClassName;
		FString Path;
		FName FunctionName;
		TWeakObjectPtr<UObject> LiveObject;
		bool bFunctionResolved = false;
	};

	struct FDelegate
	{
		FString Name;
		FString SignatureName;
		FString Kind;
		TArray<FBinding> Bindings;
	};

	struct FSource
	{
		FString Name;
		FString ClassName;
		FString Path;
		ESourceKind Kind = ESourceKind::Actor;
		TWeakObjectPtr<UObject> LiveObject;
		TArray<FDelegate> Delegates;
	};

	struct FSnapshot
	{
		FString TargetName;
		FString TargetClass;
		FString TargetPath;
		TArray<FSource> Sources;
		int32 DelegateCount = 0;
		int32 BoundDelegateCount = 0;
		int32 BindingCount = 0;
		int32 UnresolvedBindingCount = 0;
		double DurationMs = 0.0;
		bool bStale = false;
	};

	struct FVisibleDelegate
	{
		int32 DelegateIndex = INDEX_NONE;
		TArray<int32> BindingIndices;
	};

	struct FVisibleSource
	{
		int32 SourceIndex = INDEX_NONE;
		TArray<FVisibleDelegate> Delegates;
	};

	// All ordering and filtering use copied metadata. They never touch live delegates.
	void SortAndCount(FSnapshot& Snapshot);
	TArray<FVisibleSource> Filter(const FSnapshot& Snapshot, bool bBoundOnly, const FString& Search);
}
