#pragma once

#include "CoreMinimal.h"

namespace BertaDesktopCapture::Private
{
	bool IsValidFrameSize(FIntPoint Size, int32& OutPackedByteCount);

	bool CopyBgraRows(
		const uint8* Source,
		uint32 SourceRowPitch,
		FIntPoint Size,
		TArray<uint8>& OutPixels);
}
