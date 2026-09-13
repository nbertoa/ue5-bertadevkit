#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericApplication.h"

namespace UE::BertaWindowTools::Private
{
	FIntPoint CalculateCenteredPosition(const FPlatformRect& Area, FIntPoint WindowSize);
	int32 FindBestMonitorIndex(const TArray<FMonitorInfo>& Monitors, const FPlatformRect& WindowRect);
}
