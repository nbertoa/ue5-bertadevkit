#pragma once

#include "BertaSerialTypes.h"

namespace BertaSerial::Private
{
	bool NormalizePortName(const FString& PortName, FString& OutNormalizedName, int32* OutPortNumber = nullptr);
	bool PortNameLess(const FString& Left, const FString& Right);
	bool ValidateOpenOptions(
		const FBertaSerialOpenOptions& Options,
		FString& OutNormalizedPortName,
		EBertaSerialOpenError& OutError,
		FString& OutErrorMessage);
	TArray<uint8> EncodeUtf8(const FString& Text);
	FString GetLineEnding(EBertaSerialLineEnding LineEnding);
}
