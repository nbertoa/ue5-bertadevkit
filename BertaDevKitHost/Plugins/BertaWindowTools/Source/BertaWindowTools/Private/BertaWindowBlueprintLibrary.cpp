#include "BertaWindowBlueprintLibrary.h"

#include "BertaWindowGeometry.h"
#include "BertaWindowTools.h"
#include "CoreGlobals.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericApplication.h"
#include "GenericPlatform/GenericWindow.h"
#include "Widgets/SWindow.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

namespace UE::BertaWindowTools::Private
{
	struct FResolvedGameWindow
	{
		TSharedPtr<SWindow> SlateWindow;
		TSharedPtr<FGenericWindow> NativeWindow;
	};

	EBertaWindowMode ToBertaWindowMode(const EWindowMode::Type WindowMode)
	{
		switch (WindowMode)
		{
		case EWindowMode::Windowed:
			return EBertaWindowMode::Windowed;
		case EWindowMode::WindowedFullscreen:
			return EBertaWindowMode::WindowedFullscreen;
		case EWindowMode::Fullscreen:
			return EBertaWindowMode::Fullscreen;
		default:
			return EBertaWindowMode::Unknown;
		}
	}

	bool ResolveGameWindow(
		const UObject* WorldContextObject,
		FResolvedGameWindow& OutWindow,
		EBertaWindowError& OutError)
	{
		OutWindow = FResolvedGameWindow();
		OutError = EBertaWindowError::None;

		if (!IsInGameThread())
		{
			OutError = EBertaWindowError::OperationUnsupported;
			return false;
		}

		if (!WorldContextObject || !GEngine)
		{
			OutError = EBertaWindowError::InvalidWorldContext;
			return false;
		}

		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (!World)
		{
			OutError = EBertaWindowError::InvalidWorldContext;
			return false;
		}

		UGameInstance* GameInstance = World->GetGameInstance();
		if (!GameInstance)
		{
			OutError = EBertaWindowError::NoGameInstance;
			return false;
		}

		// UGameViewportClient::GetWindow() can resolve the containing Editor window for
		// embedded PIE. Runtime modules have no reliable API for distinguishing that
		// window from a dedicated PIE window, so all Editor-process access is rejected.
		if (GIsEditor)
		{
			OutError = EBertaWindowError::EmbeddedViewportUnsupported;
			return false;
		}

		UGameViewportClient* GameViewport = GameInstance->GetGameViewportClient();
		if (!GameViewport)
		{
			OutError = EBertaWindowError::NoGameViewport;
			return false;
		}

		OutWindow.SlateWindow = GameViewport->GetWindow();
		if (!OutWindow.SlateWindow.IsValid())
		{
			OutError = EBertaWindowError::NoWindow;
			return false;
		}

		OutWindow.NativeWindow = OutWindow.SlateWindow->GetNativeWindow();
		if (!OutWindow.NativeWindow.IsValid())
		{
			OutWindow = FResolvedGameWindow();
			OutError = EBertaWindowError::NoWindow;
			return false;
		}

		return true;
	}

	bool GetOuterWindowRect(const FResolvedGameWindow& Window, FPlatformRect& OutRect)
	{
#if PLATFORM_WINDOWS
		if (Window.NativeWindow->IsMinimized())
		{
			// Win32 may place an iconic window at sentinel desktop coordinates. UE's
			// restored rectangle is the meaningful outer geometry in that state.
			const FSlateRect RestoredRect = Window.SlateWindow->GetNonMaximizedRectInScreen();
			OutRect = FPlatformRect(
				FMath::RoundToInt(RestoredRect.Left),
				FMath::RoundToInt(RestoredRect.Top),
				FMath::RoundToInt(RestoredRect.Right),
				FMath::RoundToInt(RestoredRect.Bottom));
			return OutRect.Right > OutRect.Left && OutRect.Bottom > OutRect.Top;
		}

		HWND NativeHandle = static_cast<HWND>(Window.NativeWindow->GetOSWindowHandle());
		RECT NativeRect;
		if (!NativeHandle || !::GetWindowRect(NativeHandle, &NativeRect))
		{
			return false;
		}

		OutRect = FPlatformRect(NativeRect.left, NativeRect.top, NativeRect.right, NativeRect.bottom);
#else
		const FSlateRect SlateRect = Window.SlateWindow->GetRectInScreen();
		OutRect = FPlatformRect(
			FMath::RoundToInt(SlateRect.Left),
			FMath::RoundToInt(SlateRect.Top),
			FMath::RoundToInt(SlateRect.Right),
			FMath::RoundToInt(SlateRect.Bottom));
#endif
		return OutRect.Right > OutRect.Left && OutRect.Bottom > OutRect.Top;
	}

	bool GetClientSize(const FResolvedGameWindow& Window, FIntPoint& OutSize)
	{
#if PLATFORM_WINDOWS
		HWND NativeHandle = static_cast<HWND>(Window.NativeWindow->GetOSWindowHandle());
		RECT NativeRect;
		if (!NativeHandle || !::GetClientRect(NativeHandle, &NativeRect))
		{
			return false;
		}

		OutSize = FIntPoint(NativeRect.right - NativeRect.left, NativeRect.bottom - NativeRect.top);
#else
		const FVector2f ClientSize = Window.SlateWindow->GetClientSizeInScreen();
		OutSize = FIntPoint(FMath::RoundToInt(ClientSize.X), FMath::RoundToInt(ClientSize.Y));
#endif
		return OutSize.X >= 0 && OutSize.Y >= 0;
	}

	bool MoveOuterWindowTo(const FResolvedGameWindow& Window, const FIntPoint DesktopPosition)
	{
#if PLATFORM_WINDOWS
		HWND NativeHandle = static_cast<HWND>(Window.NativeWindow->GetOSWindowHandle());
		return NativeHandle
			&& ::SetWindowPos(
				NativeHandle,
				nullptr,
				DesktopPosition.X,
				DesktopPosition.Y,
				0,
				0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE) != 0;
#else
		Window.SlateWindow->MoveWindowTo(FVector2f(DesktopPosition));
		return true;
#endif
	}

	bool RequireWindowedAndRestored(const FResolvedGameWindow& Window, EBertaWindowError& OutError)
	{
		if (Window.NativeWindow->GetWindowMode() != EWindowMode::Windowed)
		{
			OutError = EBertaWindowError::WindowNotWindowed;
			return false;
		}

		if (Window.NativeWindow->IsMinimized() || Window.NativeWindow->IsMaximized())
		{
			OutError = EBertaWindowError::WindowNotRestored;
			return false;
		}

		return true;
	}

	bool ResolveGeometryWindow(
		const UObject* WorldContextObject,
		FResolvedGameWindow& OutWindow,
		EBertaWindowError& OutError)
	{
		return ResolveGameWindow(WorldContextObject, OutWindow, OutError)
			&& RequireWindowedAndRestored(OutWindow, OutError);
	}

	bool MoveWindowToArea(
		const FResolvedGameWindow& Window,
		const FPlatformRect& Area,
		EBertaWindowError& OutError)
	{
		FPlatformRect WindowRect;
		if (!GetOuterWindowRect(Window, WindowRect))
		{
			OutError = EBertaWindowError::OperationUnsupported;
			return false;
		}

		const FIntPoint WindowSize(WindowRect.Right - WindowRect.Left, WindowRect.Bottom - WindowRect.Top);
		const FIntPoint TargetPosition = CalculateCenteredPosition(Area, WindowSize);
		if (!MoveOuterWindowTo(Window, TargetPosition))
		{
			OutError = EBertaWindowError::OperationUnsupported;
			UE_LOG(LogBertaWindowTools, Warning, TEXT("Failed to move the game window to desktop position (%d, %d)."), TargetPosition.X, TargetPosition.Y);
			return false;
		}

		return true;
	}
}

bool UBertaWindowBlueprintLibrary::GetGameWindowInfo(
	const UObject* WorldContextObject,
	FBertaWindowInfo& OutInfo,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	OutInfo = FBertaWindowInfo();
	OutError = EBertaWindowError::None;

	FResolvedGameWindow Window;
	if (!ResolveGameWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}

	FPlatformRect WindowRect;
	FIntPoint ClientSize;
	if (!GetOuterWindowRect(Window, WindowRect) || !GetClientSize(Window, ClientSize))
	{
		OutError = EBertaWindowError::OperationUnsupported;
		return false;
	}

	OutInfo.Title = Window.SlateWindow->GetTitle().ToString();
	OutInfo.DesktopPosition = FIntPoint(WindowRect.Left, WindowRect.Top);
	OutInfo.WindowSize = FIntPoint(WindowRect.Right - WindowRect.Left, WindowRect.Bottom - WindowRect.Top);
	OutInfo.ClientSize = ClientSize;
	OutInfo.DPIScale = Window.NativeWindow->GetDPIScaleFactor();
	OutInfo.WindowMode = ToBertaWindowMode(Window.NativeWindow->GetWindowMode());
	OutInfo.bIsVisible = Window.NativeWindow->IsVisible();
	OutInfo.bIsActive = Window.SlateWindow->IsActive();
	OutInfo.bIsForeground = Window.NativeWindow->IsForegroundWindow();
	OutInfo.bIsMinimized = Window.NativeWindow->IsMinimized();
	OutInfo.bIsMaximized = Window.NativeWindow->IsMaximized();

	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
	const int32 MonitorIndex = FindBestMonitorIndex(DisplayMetrics.MonitorInfo, WindowRect);
	if (DisplayMetrics.MonitorInfo.IsValidIndex(MonitorIndex))
	{
		OutInfo.bHasDisplay = true;
		OutInfo.DisplayId = DisplayMetrics.MonitorInfo[MonitorIndex].ID;
	}

	return true;
}

bool UBertaWindowBlueprintLibrary::SetWindowPosition(
	const UObject* WorldContextObject,
	const FIntPoint DesktopPosition,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	FResolvedGameWindow Window;
	if (!ResolveGeometryWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}

	if (!MoveOuterWindowTo(Window, DesktopPosition))
	{
		OutError = EBertaWindowError::OperationUnsupported;
		UE_LOG(LogBertaWindowTools, Warning, TEXT("Failed to move the game window to desktop position (%d, %d)."), DesktopPosition.X, DesktopPosition.Y);
		return false;
	}

	return true;
}

bool UBertaWindowBlueprintLibrary::SetWindowClientSize(
	const UObject* WorldContextObject,
	const FIntPoint ClientSize,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	OutError = EBertaWindowError::None;
	if (ClientSize.X <= 0 || ClientSize.Y <= 0)
	{
		OutError = EBertaWindowError::InvalidSize;
		return false;
	}

	FResolvedGameWindow Window;
	if (!ResolveGeometryWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}

	// SWindow::Resize takes an already-DPI-scaled client size. Public values are
	// desktop pixels, so no Slate-unit conversion is applied here.
	Window.SlateWindow->Resize(FVector2f(ClientSize));
	return true;
}

bool UBertaWindowBlueprintLibrary::CenterWindowOnDisplay(
	const UObject* WorldContextObject,
	const FString& DisplayId,
	const bool bUseWorkArea,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	FResolvedGameWindow Window;
	if (!ResolveGeometryWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}

	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
	const FMonitorInfo* Monitor = DisplayMetrics.MonitorInfo.FindByPredicate(
		[&DisplayId](const FMonitorInfo& Candidate)
		{
			return Candidate.ID == DisplayId;
		});
	if (!Monitor)
	{
		OutError = EBertaWindowError::DisplayNotFound;
		return false;
	}

	return MoveWindowToArea(Window, bUseWorkArea ? Monitor->WorkArea : Monitor->DisplayRect, OutError);
}

bool UBertaWindowBlueprintLibrary::CenterWindowOnPrimaryDisplay(
	const UObject* WorldContextObject,
	const bool bUseWorkArea,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	FResolvedGameWindow Window;
	if (!ResolveGeometryWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}

	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
	const FMonitorInfo* Monitor = DisplayMetrics.MonitorInfo.FindByPredicate(
		[](const FMonitorInfo& Candidate)
		{
			return Candidate.bIsPrimary;
		});
	if (!Monitor)
	{
		OutError = EBertaWindowError::DisplayNotFound;
		return false;
	}

	return MoveWindowToArea(Window, bUseWorkArea ? Monitor->WorkArea : Monitor->DisplayRect, OutError);
}

bool UBertaWindowBlueprintLibrary::MaximizeWindow(
	const UObject* WorldContextObject,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	FResolvedGameWindow Window;
	if (!ResolveGameWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}
	if (Window.NativeWindow->GetWindowMode() != EWindowMode::Windowed)
	{
		OutError = EBertaWindowError::WindowNotWindowed;
		return false;
	}

	Window.SlateWindow->Maximize();
	return true;
}

bool UBertaWindowBlueprintLibrary::MinimizeWindow(
	const UObject* WorldContextObject,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	FResolvedGameWindow Window;
	if (!ResolveGameWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}
	if (Window.NativeWindow->GetWindowMode() != EWindowMode::Windowed)
	{
		OutError = EBertaWindowError::WindowNotWindowed;
		return false;
	}

	Window.SlateWindow->Minimize();
	return true;
}

bool UBertaWindowBlueprintLibrary::RestoreWindow(
	const UObject* WorldContextObject,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	FResolvedGameWindow Window;
	if (!ResolveGameWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}
	if (Window.NativeWindow->GetWindowMode() != EWindowMode::Windowed)
	{
		OutError = EBertaWindowError::WindowNotWindowed;
		return false;
	}

	Window.SlateWindow->Restore();
	return true;
}

bool UBertaWindowBlueprintLibrary::BringWindowToFront(
	const UObject* WorldContextObject,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	FResolvedGameWindow Window;
	if (!ResolveGameWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}
	if (Window.NativeWindow->IsMinimized())
	{
		OutError = EBertaWindowError::WindowNotRestored;
		return false;
	}

	Window.SlateWindow->BringToFront(false);
	Window.NativeWindow->SetWindowFocus();
	return true;
}

bool UBertaWindowBlueprintLibrary::SetWindowTitle(
	const UObject* WorldContextObject,
	const FString& Title,
	EBertaWindowError& OutError)
{
	using namespace UE::BertaWindowTools::Private;

	FResolvedGameWindow Window;
	if (!ResolveGameWindow(WorldContextObject, Window, OutError))
	{
		return false;
	}

	Window.SlateWindow->SetTitle(FText::FromString(Title));
	return true;
}
