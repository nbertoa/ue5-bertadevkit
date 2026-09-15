#pragma once

#include "TickGraph/BertaTickGraphSnapshot.h"

class AActor;

namespace BertaTickGraph
{
	bool Capture(AActor* Target, FSnapshot& OutSnapshot, FString& Error);
}
