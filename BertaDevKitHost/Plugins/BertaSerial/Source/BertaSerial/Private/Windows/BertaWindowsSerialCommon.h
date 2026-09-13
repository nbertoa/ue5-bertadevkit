#pragma once

#include "CoreMinimal.h"

namespace BertaSerial::Windows
{
	FString FormatWindowsError(uint32 ErrorCode);
	bool IsConnectionLostError(uint32 ErrorCode);
}
