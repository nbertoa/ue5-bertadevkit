#pragma once

#include "ObjectGraph/BertaObjectGraphSnapshot.h"

namespace BertaObjectGraph
{
	// Runs entirely on the Editor game thread. Neither this class nor its
	// result owns a runtime UObject. Selection is restored before return.
	bool Analyze(TWeakObjectPtr<UObject> Target, FSnapshot& OutSnapshot, FString& OutError);
}
