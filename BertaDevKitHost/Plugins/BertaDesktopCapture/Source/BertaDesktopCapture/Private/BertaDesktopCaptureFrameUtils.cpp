#include "BertaDesktopCaptureFrameUtils.h"

namespace BertaDesktopCapture::Private
{
	bool IsValidFrameSize(const FIntPoint Size, int32& OutPackedByteCount)
	{
		OutPackedByteCount = 0;
		if (Size.X <= 0 || Size.Y <= 0)
		{
			return false;
		}

		constexpr int64 BytesPerPixel = 4;
		const int64 ByteCount = static_cast<int64>(Size.X) * static_cast<int64>(Size.Y) * BytesPerPixel;
		if (ByteCount > MAX_int32)
		{
			return false;
		}

		OutPackedByteCount = static_cast<int32>(ByteCount);
		return true;
	}

	bool CopyBgraRows(
		const uint8* Source,
		const uint32 SourceRowPitch,
		const FIntPoint Size,
		TArray<uint8>& OutPixels)
	{
		OutPixels.Reset();

		int32 PackedByteCount = 0;
		if (Source == nullptr || !IsValidFrameSize(Size, PackedByteCount))
		{
			return false;
		}

		const uint32 PackedRowBytes = static_cast<uint32>(Size.X) * 4u;
		if (SourceRowPitch < PackedRowBytes)
		{
			return false;
		}

		OutPixels.SetNumUninitialized(PackedByteCount);
		for (int32 Row = 0; Row < Size.Y; ++Row)
		{
			FMemory::Memcpy(
				OutPixels.GetData() + static_cast<SIZE_T>(Row) * PackedRowBytes,
				Source + static_cast<SIZE_T>(Row) * SourceRowPitch,
				PackedRowBytes);
		}
		return true;
	}
}
