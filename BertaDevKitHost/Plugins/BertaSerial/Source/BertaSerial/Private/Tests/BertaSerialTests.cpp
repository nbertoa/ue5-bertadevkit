#if WITH_DEV_AUTOMATION_TESTS

#include "BertaSerialUtils.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaSerialPortNameTest,
	"BertaSerial.Validation.PortNamesAndSort",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaSerialPortNameTest::RunTest(const FString& Parameters)
{
	FString Normalized;
	int32 Number = 0;
	TestTrue(TEXT("COM1 is valid"), BertaSerial::Private::NormalizePortName(TEXT("COM1"), Normalized, &Number));
	TestEqual(TEXT("COM1 normalized"), Normalized, FString(TEXT("COM1")));
	TestEqual(TEXT("COM1 number"), Number, 1);
	TestTrue(TEXT("com2 is valid"), BertaSerial::Private::NormalizePortName(TEXT("com2"), Normalized));
	TestEqual(TEXT("com2 normalized"), Normalized, FString(TEXT("COM2")));
	TestTrue(TEXT("CoM10 is valid"), BertaSerial::Private::NormalizePortName(TEXT("CoM10"), Normalized));
	TestEqual(TEXT("CoM10 normalized"), Normalized, FString(TEXT("COM10")));

	const TArray<FString> InvalidNames = {
		TEXT("COM"), TEXT("COM0"), TEXT("COM-1"), TEXT("LPT1"),
		TEXT("ABC"), TEXT("\\\\.\\COM3"), TEXT("COM3foo")
	};
	for (const FString& InvalidName : InvalidNames)
	{
		TestFalse(*FString::Printf(TEXT("%s is invalid"), *InvalidName),
			BertaSerial::Private::NormalizePortName(InvalidName, Normalized));
	}

	TArray<FString> PortNames = { TEXT("COM10"), TEXT("COM2"), TEXT("COM1"), TEXT("COM20"), TEXT("COM3") };
	PortNames.Sort([](const FString& Left, const FString& Right)
	{
		return BertaSerial::Private::PortNameLess(Left, Right);
	});
	const TArray<FString> Expected = { TEXT("COM1"), TEXT("COM2"), TEXT("COM3"), TEXT("COM10"), TEXT("COM20") };
	TestEqual(TEXT("Natural COM-port order"), PortNames, Expected);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaSerialSettingsTest,
	"BertaSerial.Validation.OpenSettings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaSerialSettingsTest::RunTest(const FString& Parameters)
{
	auto IsValid = [](const int32 DataBits, const EBertaSerialStopBits StopBits, const int32 BaudRate = 115200)
	{
		FBertaSerialOpenOptions Options;
		Options.PortName = TEXT("COM1");
		Options.DataBits = DataBits;
		Options.StopBits = StopBits;
		Options.BaudRate = BaudRate;
		FString Normalized;
		FString Message;
		EBertaSerialOpenError Error = EBertaSerialOpenError::None;
		return BertaSerial::Private::ValidateOpenOptions(Options, Normalized, Error, Message);
	};

	TestTrue(TEXT("8-N-1"), IsValid(8, EBertaSerialStopBits::One));
	FBertaSerialOpenOptions SevenEvenOne;
	SevenEvenOne.PortName = TEXT("COM1");
	SevenEvenOne.DataBits = 7;
	SevenEvenOne.Parity = EBertaSerialParity::Even;
	FString Normalized;
	FString Message;
	EBertaSerialOpenError Error = EBertaSerialOpenError::None;
	TestTrue(
		TEXT("7-E-1"),
		BertaSerial::Private::ValidateOpenOptions(SevenEvenOne, Normalized, Error, Message));
	TestTrue(TEXT("5-N-1.5"), IsValid(5, EBertaSerialStopBits::OnePointFive));
	TestFalse(TEXT("Four data bits"), IsValid(4, EBertaSerialStopBits::One));
	TestFalse(TEXT("Nine data bits"), IsValid(9, EBertaSerialStopBits::One));
	TestFalse(TEXT("Five data bits with two stop bits"), IsValid(5, EBertaSerialStopBits::Two));
	TestFalse(TEXT("Eight data bits with 1.5 stop bits"), IsValid(8, EBertaSerialStopBits::OnePointFive));
	TestFalse(TEXT("Zero baud"), IsValid(8, EBertaSerialStopBits::One, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaSerialUtf8Test,
	"BertaSerial.Encoding.Utf8AndLineEndings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaSerialUtf8Test::RunTest(const FString& Parameters)
{
	const TArray<uint8> Hello = BertaSerial::Private::EncodeUtf8(TEXT("Hello"));
	const TArray<uint8> ExpectedHello = { 'H', 'e', 'l', 'l', 'o' };
	TestEqual(TEXT("UTF-8 has no null terminator"), Hello, ExpectedHello);

	const TArray<uint8> NonAscii = BertaSerial::Private::EncodeUtf8(TEXT("ñ"));
	const TArray<uint8> ExpectedNonAscii = { 0xC3, 0xB1 };
	TestEqual(TEXT("Non-ASCII UTF-8"), NonAscii, ExpectedNonAscii);
	TestEqual(TEXT("LF"), BertaSerial::Private::GetLineEnding(EBertaSerialLineEnding::LF), FString(TEXT("\n")));
	TestEqual(TEXT("CRLF"), BertaSerial::Private::GetLineEnding(EBertaSerialLineEnding::CRLF), FString(TEXT("\r\n")));
	TestEqual(TEXT("CR"), BertaSerial::Private::GetLineEnding(EBertaSerialLineEnding::CR), FString(TEXT("\r")));
	return true;
}

#endif
