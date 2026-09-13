#include "Windows/BertaWindowsSerialCommon.h"

#include "Windows/WindowsHWrapper.h"

namespace BertaSerial::Windows
{
	FString FormatWindowsError(const uint32 ErrorCode)
	{
		WCHAR* MessageBuffer = nullptr;
		const DWORD Length = ::FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			ErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<WCHAR*>(&MessageBuffer),
			0,
			nullptr);
		FString Message = Length > 0 && MessageBuffer != nullptr
			? FString(static_cast<int32>(Length), MessageBuffer)
			: FString::Printf(TEXT("Windows error %u"), ErrorCode);
		if (MessageBuffer != nullptr)
		{
			::LocalFree(MessageBuffer);
		}
		Message.TrimStartAndEndInline();
		return FString::Printf(TEXT("%s (error %u)"), *Message, ErrorCode);
	}

	bool IsConnectionLostError(const uint32 ErrorCode)
	{
		return ErrorCode == ERROR_DEVICE_NOT_CONNECTED
			|| ErrorCode == ERROR_DEV_NOT_EXIST
			|| ErrorCode == ERROR_FILE_NOT_FOUND
			|| ErrorCode == ERROR_PATH_NOT_FOUND
			|| ErrorCode == ERROR_INVALID_HANDLE;
	}
}
