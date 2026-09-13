#include "BertaSerialUtils.h"

#include "Containers/StringConv.h"

namespace BertaSerial::Private
{
	bool NormalizePortName(const FString& PortName, FString& OutNormalizedName, int32* OutPortNumber)
	{
		OutNormalizedName.Reset();
		if (OutPortNumber != nullptr)
		{
			*OutPortNumber = 0;
		}
		if (PortName.Len() <= 3 || !PortName.Left(3).Equals(TEXT("COM"), ESearchCase::IgnoreCase))
		{
			return false;
		}

		const FString NumberText = PortName.RightChop(3);
		int32 PortNumber = 0;
		for (const TCHAR Character : NumberText)
		{
			if (Character < TEXT('0') || Character > TEXT('9'))
			{
				return false;
			}
			const int32 Digit = Character - TEXT('0');
			if (PortNumber > (MAX_int32 - Digit) / 10)
			{
				return false;
			}
			PortNumber = PortNumber * 10 + Digit;
		}

		if (PortNumber <= 0)
		{
			return false;
		}

		OutNormalizedName = FString::Printf(TEXT("COM%d"), PortNumber);
		if (OutPortNumber != nullptr)
		{
			*OutPortNumber = PortNumber;
		}
		return true;
	}

	bool PortNameLess(const FString& Left, const FString& Right)
	{
		FString Ignored;
		int32 LeftNumber = 0;
		int32 RightNumber = 0;
		NormalizePortName(Left, Ignored, &LeftNumber);
		NormalizePortName(Right, Ignored, &RightNumber);
		return LeftNumber < RightNumber;
	}

	bool ValidateOpenOptions(
		const FBertaSerialOpenOptions& Options,
		FString& OutNormalizedPortName,
		EBertaSerialOpenError& OutError,
		FString& OutErrorMessage)
	{
		OutNormalizedPortName.Reset();
		OutError = EBertaSerialOpenError::None;
		OutErrorMessage.Reset();
		if (!NormalizePortName(Options.PortName, OutNormalizedPortName))
		{
			OutError = EBertaSerialOpenError::InvalidPortName;
			OutErrorMessage = TEXT("PortName must use the COM<number> form with a positive decimal number.");
			return false;
		}
		if (Options.BaudRate <= 0)
		{
			OutError = EBertaSerialOpenError::InvalidSettings;
			OutErrorMessage = TEXT("BaudRate must be greater than zero.");
			return false;
		}
		if (Options.DataBits < 5 || Options.DataBits > 8)
		{
			OutError = EBertaSerialOpenError::InvalidSettings;
			OutErrorMessage = TEXT("DataBits must be between 5 and 8 inclusive.");
			return false;
		}
		if (Options.DataBits == 5 && Options.StopBits == EBertaSerialStopBits::Two)
		{
			OutError = EBertaSerialOpenError::InvalidSettings;
			OutErrorMessage = TEXT("Windows serial ports do not support two stop bits with five data bits.");
			return false;
		}
		if (Options.DataBits != 5 && Options.StopBits == EBertaSerialStopBits::OnePointFive)
		{
			OutError = EBertaSerialOpenError::InvalidSettings;
			OutErrorMessage = TEXT("Windows serial ports only support one-and-a-half stop bits with five data bits.");
			return false;
		}
		if (Options.Parity < EBertaSerialParity::None || Options.Parity > EBertaSerialParity::Space
			|| Options.StopBits < EBertaSerialStopBits::One || Options.StopBits > EBertaSerialStopBits::Two
			|| Options.FlowControl < EBertaSerialFlowControl::None || Options.FlowControl > EBertaSerialFlowControl::XOnXOff)
		{
			OutError = EBertaSerialOpenError::InvalidSettings;
			OutErrorMessage = TEXT("Parity, StopBits, or FlowControl contains an unsupported value.");
			return false;
		}
		return true;
	}

	TArray<uint8> EncodeUtf8(const FString& Text)
	{
		const auto Converted = StringCast<UTF8CHAR>(*Text, Text.Len());
		TArray<uint8> Bytes;
		Bytes.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
		return Bytes;
	}

	FString GetLineEnding(const EBertaSerialLineEnding LineEnding)
	{
		switch (LineEnding)
		{
		case EBertaSerialLineEnding::LF:
			return TEXT("\n");
		case EBertaSerialLineEnding::CRLF:
			return TEXT("\r\n");
		case EBertaSerialLineEnding::CR:
			return TEXT("\r");
		default:
			return FString();
		}
	}
}
