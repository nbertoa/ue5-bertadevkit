#if WITH_DEV_AUTOMATION_TESTS

#include "BertaDesktopCaptureFrameUtils.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBertaDesktopCaptureRowCopyTest,
	"BertaDesktopCapture.FrameCopy.RespectsRowPitch",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FBertaDesktopCaptureRowCopyTest::RunTest(const FString& Parameters)
{
	const TArray<uint8> Source = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 200, 201, 202, 203,
		12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 204, 205, 206, 207
	};

	TArray<uint8> Actual;
	TestTrue(
		TEXT("Padded rows copy successfully"),
		BertaDesktopCapture::Private::CopyBgraRows(Source.GetData(), 16, FIntPoint(3, 2), Actual));
	TestEqual(TEXT("Packed output byte count"), Actual.Num(), 24);

	TArray<uint8> Expected;
	Expected.Append(Source.GetData(), 12);
	Expected.Append(Source.GetData() + 16, 12);
	TestTrue(TEXT("Padding bytes are omitted and row order is preserved"), Actual == Expected);
	return true;
}

#endif
