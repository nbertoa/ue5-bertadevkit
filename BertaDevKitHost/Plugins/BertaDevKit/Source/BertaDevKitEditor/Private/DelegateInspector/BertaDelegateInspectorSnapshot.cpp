#include "DelegateInspector/BertaDelegateInspectorSnapshot.h"

namespace BertaDelegateInspector
{
	void SortAndCount(FSnapshot& Snapshot)
	{
		Snapshot.Sources.Sort([](const FSource& A, const FSource& B)
		{
			if (A.Kind != B.Kind) { return A.Kind == ESourceKind::Actor; }
			const int32 Folded = A.Path.Compare(B.Path, ESearchCase::IgnoreCase);
			return Folded != 0 ? Folded < 0 : A.Path.Compare(B.Path, ESearchCase::CaseSensitive) < 0;
		});
		Snapshot.DelegateCount = 0;
		Snapshot.BoundDelegateCount = 0;
		Snapshot.BindingCount = 0;
		Snapshot.UnresolvedBindingCount = 0;
		for (FSource& Source : Snapshot.Sources)
		{
			Source.Delegates.Sort([](const FDelegate& A, const FDelegate& B)
			{
				const int32 Folded = A.Name.Compare(B.Name, ESearchCase::IgnoreCase);
				return Folded != 0 ? Folded < 0 : A.Name.Compare(B.Name, ESearchCase::CaseSensitive) < 0;
			});
			for (FDelegate& Delegate : Source.Delegates)
			{
				Delegate.Bindings.Sort([](const FBinding& A, const FBinding& B)
				{
					const int32 PathOrder = A.Path.Compare(B.Path, ESearchCase::IgnoreCase);
					if (PathOrder != 0) { return PathOrder < 0; }
					const int32 ExactPathOrder = A.Path.Compare(B.Path, ESearchCase::CaseSensitive);
					if (ExactPathOrder != 0) { return ExactPathOrder < 0; }
					return A.FunctionName.LexicalLess(B.FunctionName);
				});
				++Snapshot.DelegateCount;
				Snapshot.BindingCount += Delegate.Bindings.Num();
				for (const FBinding& Binding : Delegate.Bindings)
				{
					if (!Binding.bFunctionResolved) { ++Snapshot.UnresolvedBindingCount; }
				}
				if (!Delegate.Bindings.IsEmpty()) { ++Snapshot.BoundDelegateCount; }
			}
		}
	}

	static bool Matches(const FString& Value, const FString& Search)
	{
		return Value.Contains(Search, ESearchCase::IgnoreCase);
	}

	TArray<FVisibleSource> Filter(const FSnapshot& Snapshot, bool bBoundOnly, const FString& Search)
	{
		TArray<FVisibleSource> Result;
		for (int32 SourceIndex = 0; SourceIndex < Snapshot.Sources.Num(); ++SourceIndex)
		{
			const FSource& Source = Snapshot.Sources[SourceIndex];
			const bool bSourceMatches = Search.IsEmpty() || Matches(Source.Name, Search) || Matches(Source.ClassName, Search);
			FVisibleSource VisibleSource;
			VisibleSource.SourceIndex = SourceIndex;
			for (int32 DelegateIndex = 0; DelegateIndex < Source.Delegates.Num(); ++DelegateIndex)
			{
				const FDelegate& Delegate = Source.Delegates[DelegateIndex];
				if (bBoundOnly && Delegate.Bindings.IsEmpty()) { continue; }
				const bool bDelegateMatches = bSourceMatches || Matches(Delegate.Name, Search);
				FVisibleDelegate VisibleDelegate;
				VisibleDelegate.DelegateIndex = DelegateIndex;
				for (int32 BindingIndex = 0; BindingIndex < Delegate.Bindings.Num(); ++BindingIndex)
				{
					const FBinding& Binding = Delegate.Bindings[BindingIndex];
					if (bDelegateMatches || Matches(Binding.Name, Search) || Matches(Binding.ClassName, Search)
						|| Matches(Binding.FunctionName.ToString(), Search))
					{
						VisibleDelegate.BindingIndices.Add(BindingIndex);
					}
				}
				if (bDelegateMatches || !VisibleDelegate.BindingIndices.IsEmpty())
				{
					VisibleSource.Delegates.Add(MoveTemp(VisibleDelegate));
				}
			}
			if (!VisibleSource.Delegates.IsEmpty()) { Result.Add(MoveTemp(VisibleSource)); }
		}
		return Result;
	}
}
