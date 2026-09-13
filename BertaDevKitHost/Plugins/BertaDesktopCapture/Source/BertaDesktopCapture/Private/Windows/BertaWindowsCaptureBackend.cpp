#include "Windows/BertaWindowsCaptureBackend.h"

#include "BertaDesktopCapture.h"
#include "BertaDesktopCaptureDispatcher.h"
#include "BertaDesktopCaptureFrameUtils.h"
#include "GenericPlatform/GenericApplication.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/ScopeLock.h"

#include "Windows/WindowsHWrapper.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include <d3d11.h>
#include <dwmapi.h>
#include <roapi.h>
#include <windows.foundation.h>
#include <windows.graphics.capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <wrl.h>
#include <wrl/event.h>
#include <wrl/wrappers/corewrappers.h>
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"

namespace BertaDesktopCapture::Private
{
	using ABI::Windows::Foundation::IClosable;
	using ABI::Windows::Graphics::Capture::IDirect3D11CaptureFrame;
	using ABI::Windows::Graphics::Capture::IDirect3D11CaptureFramePool;
	using ABI::Windows::Graphics::Capture::IDirect3D11CaptureFramePoolStatics2;
	using ABI::Windows::Graphics::Capture::IGraphicsCaptureItem;
	using ABI::Windows::Graphics::Capture::IGraphicsCaptureSession;
	using ABI::Windows::Graphics::Capture::IGraphicsCaptureSessionStatics;
	using ABI::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice;
	using ABI::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface;
	using Microsoft::WRL::Callback;
	using Microsoft::WRL::ComPtr;
	using Microsoft::WRL::Wrappers::HStringReference;

	using FFrameArrivedHandler =
		ABI::Windows::Foundation::__FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectable_t;
	using FItemClosedHandler =
		ABI::Windows::Foundation::__FITypedEventHandler_2_Windows__CGraphics__CCapture__CGraphicsCaptureItem_IInspectable_t;

	constexpr TCHAR WindowIdPrefix[] = TEXT("BertaDesktopCapture.Window.v1:");
	constexpr int32 CaptureBufferCount = 2;

	void CloseInspectable(IInspectable* Object)
	{
		if (Object == nullptr)
		{
			return;
		}

		ComPtr<IClosable> Closable;
		if (SUCCEEDED(Object->QueryInterface(IID_PPV_ARGS(&Closable))))
		{
			Closable->Close();
		}
	}

	template <typename T>
	HRESULT GetActivationFactory(const wchar_t* RuntimeClassName, ComPtr<T>& OutFactory)
	{
		return RoGetActivationFactory(
			HStringReference(RuntimeClassName).Get(),
			IID_PPV_ARGS(&OutFactory));
	}

	class FScopedRoInitialize final
	{
	public:
		FScopedRoInitialize()
		{
			Result = RoInitialize(RO_INIT_MULTITHREADED);
			bNeedsUninitialize = Result == S_OK || Result == S_FALSE;
		}

		~FScopedRoInitialize()
		{
			if (bNeedsUninitialize)
			{
				RoUninitialize();
			}
		}

		bool IsUsable() const
		{
			return SUCCEEDED(Result) || Result == RPC_E_CHANGED_MODE;
		}

	private:
		HRESULT Result = E_FAIL;
		bool bNeedsUninitialize = false;
	};

	bool IsHexString(const FString& Value)
	{
		if (Value.IsEmpty())
		{
			return false;
		}
		for (const TCHAR Character : Value)
		{
			if (!FChar::IsHexDigit(Character))
			{
				return false;
			}
		}
		return true;
	}

	FString EncodeWindowId(const HWND Window)
	{
		return FString::Printf(
			TEXT("%s%08X:%016llX"),
			WindowIdPrefix,
			FPlatformProcess::GetCurrentProcessId(),
			static_cast<uint64>(reinterpret_cast<UPTRINT>(Window)));
	}

	bool DecodeWindowId(const FString& Id, HWND& OutWindow)
	{
		OutWindow = nullptr;
		if (!Id.StartsWith(WindowIdPrefix, ESearchCase::CaseSensitive))
		{
			return false;
		}

		TArray<FString> Parts;
		Id.RightChop(UE_ARRAY_COUNT(WindowIdPrefix) - 1).ParseIntoArray(Parts, TEXT(":"), false);
		if (Parts.Num() != 2 || Parts[0].Len() != 8 || Parts[1].Len() != 16
			|| !IsHexString(Parts[0]) || !IsHexString(Parts[1]))
		{
			return false;
		}

		const uint64 ProcessToken = FCString::Strtoui64(*Parts[0], nullptr, 16);
		const uint64 HandleValue = FCString::Strtoui64(*Parts[1], nullptr, 16);
		if (ProcessToken != FPlatformProcess::GetCurrentProcessId() || HandleValue == 0)
		{
			return false;
		}

		OutWindow = reinterpret_cast<HWND>(static_cast<UPTRINT>(HandleValue));
		return ::IsWindow(OutWindow) != 0;
	}

	bool ResolveDisplay(const FString& DisplayId, HMONITOR& OutMonitor)
	{
		OutMonitor = nullptr;
		FDisplayMetrics Metrics;
		FDisplayMetrics::RebuildDisplayMetrics(Metrics);
		const FMonitorInfo* Monitor = Metrics.MonitorInfo.FindByPredicate(
			[&DisplayId](const FMonitorInfo& Candidate)
			{
				return Candidate.ID == DisplayId;
			});
		if (Monitor == nullptr || Monitor->NativeHandle == nullptr)
		{
			return false;
		}

		OutMonitor = static_cast<HMONITOR>(Monitor->NativeHandle);
		return true;
	}

	bool CheckRuntimeSupport()
	{
		FScopedRoInitialize RoScope;
		if (!RoScope.IsUsable())
		{
			return false;
		}

		ComPtr<IGraphicsCaptureSessionStatics> SessionStatics;
		boolean bSupported = false;
		if (FAILED(GetActivationFactory(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureSession, SessionStatics))
			|| FAILED(SessionStatics->IsSupported(&bSupported)) || !bSupported)
		{
			return false;
		}

		ComPtr<IGraphicsCaptureItemInterop> ItemInterop;
		ComPtr<IDirect3D11CaptureFramePoolStatics2> FramePoolStatics;
		return SUCCEEDED(GetActivationFactory(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureItem, ItemInterop))
			&& SUCCEEDED(GetActivationFactory(RuntimeClass_Windows_Graphics_Capture_Direct3D11CaptureFramePool, FramePoolStatics));
	}

	BOOL CALLBACK EnumerateWindowCallback(const HWND Window, const LPARAM Parameter)
	{
		TArray<FBertaDesktopCaptureSource>& Sources =
			*reinterpret_cast<TArray<FBertaDesktopCaptureSource>*>(Parameter);
		if (!::IsWindow(Window) || !::IsWindowVisible(Window))
		{
			return 1;
		}

		DWORD bCloaked = 0;
		if (SUCCEEDED(::DwmGetWindowAttribute(Window, DWMWA_CLOAKED, &bCloaked, sizeof(bCloaked))) && bCloaked != 0)
		{
			return 1;
		}

		const int32 TitleLength = ::GetWindowTextLengthW(Window);
		if (TitleLength <= 0)
		{
			return 1;
		}

		TArray<WCHAR> TitleBuffer;
		TitleBuffer.SetNumZeroed(TitleLength + 1);
		if (::GetWindowTextW(Window, TitleBuffer.GetData(), TitleBuffer.Num()) <= 0)
		{
			return 1;
		}

		FString Title(TitleBuffer.GetData());
		Title.TrimStartAndEndInline();
		RECT Rect;
		if (Title.IsEmpty() || !::GetWindowRect(Window, &Rect)
			|| Rect.right <= Rect.left || Rect.bottom <= Rect.top)
		{
			return 1;
		}

		FBertaDesktopCaptureSource& Source = Sources.AddDefaulted_GetRef();
		Source.Type = EBertaDesktopCaptureSourceType::Window;
		Source.Id = EncodeWindowId(Window);
		Source.Name = MoveTemp(Title);
		Source.Size = FIntPoint(Rect.right - Rect.left, Rect.bottom - Rect.top);
		return 1;
	}

	class FCaptureCallbackState final
		: public TSharedFromThis<FCaptureCallbackState, ESPMode::ThreadSafe>
	{
	public:
		FCaptureCallbackState(
			const int32 InMaxFrameRate,
			const FIntPoint InInitialSize,
			const TSharedRef<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe>& InDispatcher)
			: Dispatcher(InDispatcher)
			, CurrentSize(InInitialSize)
			, MinimumFrameInterval(1.0 / static_cast<double>(InMaxFrameRate))
		{
		}

		void SetResources(
			const ComPtr<ID3D11Device>& InDevice,
			const ComPtr<ID3D11DeviceContext>& InContext,
			const ComPtr<IDirect3DDevice>& InDirect3DDevice,
			const ComPtr<IDirect3D11CaptureFramePool>& InFramePool)
		{
			FScopeLock Lock(&Mutex);
			Device = InDevice;
			Context = InContext;
			Direct3DDevice = InDirect3DDevice;
			FramePool = InFramePool;
			bActive = true;
		}

		HRESULT OnFrameArrived()
		{
			FScopeLock Lock(&Mutex);
			if (!bActive || bTerminalSignaled || !FramePool)
			{
				return S_OK;
			}

			ComPtr<IDirect3D11CaptureFrame> Frame;
			HRESULT Result = FramePool->TryGetNextFrame(&Frame);
			if (FAILED(Result))
			{
				SignalFailureLocked(TEXT("Windows Graphics Capture could not retrieve the next frame."));
				return S_OK;
			}
			if (!Frame)
			{
				return S_OK;
			}

			ABI::Windows::Graphics::SizeInt32 ContentSize{};
			Result = Frame->get_ContentSize(&ContentSize);
			if (FAILED(Result) || ContentSize.Width <= 0 || ContentSize.Height <= 0)
			{
				CloseInspectable(Frame.Get());
				SignalFailureLocked(TEXT("Windows Graphics Capture returned an invalid frame size."));
				return S_OK;
			}

			const FIntPoint NewSize(ContentSize.Width, ContentSize.Height);
			if (NewSize != CurrentSize)
			{
				CloseInspectable(Frame.Get());
				Frame.Reset();
				StagingTexture.Reset();
				Result = FramePool->Recreate(
					Direct3DDevice.Get(),
					ABI::Windows::Graphics::DirectX::DirectXPixelFormat_B8G8R8A8UIntNormalized,
					CaptureBufferCount,
					ContentSize);
				if (FAILED(Result))
				{
					SignalFailureLocked(TEXT("Windows Graphics Capture could not recreate its frame pool after a source resize."));
					return S_OK;
				}

				CurrentSize = NewSize;
				bHasAcceptedFrame = false;
				if (TSharedPtr<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe> Target = Dispatcher.Pin())
				{
					Target->PublishSizeChanged(NewSize);
				}
				return S_OK;
			}

			const double Now = FPlatformTime::Seconds();
			if (bHasAcceptedFrame && Now - LastAcceptedFrameTime < MinimumFrameInterval)
			{
				CloseInspectable(Frame.Get());
				return S_OK;
			}

			ComPtr<IDirect3DSurface> Surface;
			ComPtr<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess> Access;
			ComPtr<ID3D11Texture2D> FrameTexture;
			Result = Frame->get_Surface(&Surface);
			if (SUCCEEDED(Result))
			{
				Result = Surface.As(&Access);
			}
			if (SUCCEEDED(Result))
			{
				Result = Access->GetInterface(IID_PPV_ARGS(&FrameTexture));
			}
			if (FAILED(Result) || !FrameTexture)
			{
				CloseInspectable(Frame.Get());
				SignalFailureLocked(TEXT("Windows Graphics Capture could not access the D3D11 frame texture."));
				return S_OK;
			}

			D3D11_TEXTURE2D_DESC FrameDescription{};
			FrameTexture->GetDesc(&FrameDescription);
			if (FrameDescription.Width != static_cast<uint32>(CurrentSize.X)
				|| FrameDescription.Height != static_cast<uint32>(CurrentSize.Y)
				|| FrameDescription.Format != DXGI_FORMAT_B8G8R8A8_UNORM)
			{
				CloseInspectable(Frame.Get());
				SignalFailureLocked(TEXT("Windows Graphics Capture returned an unexpected D3D11 texture format or size."));
				return S_OK;
			}

			if (!EnsureStagingTextureLocked(FrameDescription))
			{
				CloseInspectable(Frame.Get());
				SignalFailureLocked(TEXT("Failed to create the D3D11 staging texture used for desktop capture readback."));
				return S_OK;
			}

			Context->CopyResource(StagingTexture.Get(), FrameTexture.Get());
			D3D11_MAPPED_SUBRESOURCE Mapped{};
			Result = Context->Map(StagingTexture.Get(), 0, D3D11_MAP_READ, 0, &Mapped);
			if (FAILED(Result))
			{
				CloseInspectable(Frame.Get());
				SignalFailureLocked(TEXT("Failed to map the D3D11 staging texture used for desktop capture readback."));
				return S_OK;
			}

			TArray<uint8> Pixels;
			const bool bCopied = CopyBgraRows(
				static_cast<const uint8*>(Mapped.pData),
				Mapped.RowPitch,
				CurrentSize,
				Pixels);
			Context->Unmap(StagingTexture.Get(), 0);
			CloseInspectable(Frame.Get());
			if (!bCopied)
			{
				SignalFailureLocked(TEXT("Desktop capture frame dimensions or row pitch were invalid."));
				return S_OK;
			}

			bHasAcceptedFrame = true;
			LastAcceptedFrameTime = Now;
			if (TSharedPtr<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe> Target = Dispatcher.Pin())
			{
				Target->PublishFrame(CurrentSize, MoveTemp(Pixels));
			}
			return S_OK;
		}

		HRESULT OnItemClosed()
		{
			FScopeLock Lock(&Mutex);
			if (!bActive || bTerminalSignaled)
			{
				return S_OK;
			}

			bTerminalSignaled = true;
			if (TSharedPtr<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe> Target = Dispatcher.Pin())
			{
				Target->PublishStopped(EBertaDesktopCaptureStopReason::SourceClosed, FString());
			}
			return S_OK;
		}

		void Deactivate()
		{
			FScopeLock Lock(&Mutex);
			bActive = false;
			Dispatcher.Reset();
			StagingTexture.Reset();
			FramePool.Reset();
			Direct3DDevice.Reset();
			Context.Reset();
			Device.Reset();
		}

	private:
		bool EnsureStagingTextureLocked(const D3D11_TEXTURE2D_DESC& FrameDescription)
		{
			if (StagingTexture)
			{
				return true;
			}

			D3D11_TEXTURE2D_DESC StagingDescription = FrameDescription;
			StagingDescription.BindFlags = 0;
			StagingDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			StagingDescription.MiscFlags = 0;
			StagingDescription.Usage = D3D11_USAGE_STAGING;
			return SUCCEEDED(Device->CreateTexture2D(&StagingDescription, nullptr, &StagingTexture));
		}

		void SignalFailureLocked(FString&& Message)
		{
			if (bTerminalSignaled)
			{
				return;
			}

			bTerminalSignaled = true;
			if (TSharedPtr<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe> Target = Dispatcher.Pin())
			{
				Target->PublishStopped(EBertaDesktopCaptureStopReason::CaptureFailed, MoveTemp(Message));
			}
		}

		FCriticalSection Mutex;
		TWeakPtr<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe> Dispatcher;
		ComPtr<ID3D11Device> Device;
		ComPtr<ID3D11DeviceContext> Context;
		ComPtr<IDirect3DDevice> Direct3DDevice;
		ComPtr<IDirect3D11CaptureFramePool> FramePool;
		ComPtr<ID3D11Texture2D> StagingTexture;
		FIntPoint CurrentSize;
		double MinimumFrameInterval = 0.0;
		double LastAcceptedFrameTime = 0.0;
		bool bHasAcceptedFrame = false;
		bool bActive = false;
		bool bTerminalSignaled = false;
	};
}

class FBertaWindowsCaptureBackend::FImplementation final
{
public:
	~FImplementation()
	{
		Stop();
	}

	bool Initialize(
		const FBertaDesktopCaptureSource& Source,
		const int32 MaxFrameRate,
		const TSharedRef<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe>& Dispatcher,
		FIntPoint& OutInitialSize,
		EBertaDesktopCaptureError& OutError,
		FString& OutErrorMessage)
	{
		using namespace BertaDesktopCapture::Private;

		check(IsInGameThread());
		OutInitialSize = FIntPoint::ZeroValue;
		OutError = EBertaDesktopCaptureError::CaptureInitializationFailed;
		OutErrorMessage.Reset();

		const HRESULT RoResult = RoInitialize(RO_INIT_MULTITHREADED);
		if (FAILED(RoResult) && RoResult != RPC_E_CHANGED_MODE)
		{
			OutError = EBertaDesktopCaptureError::UnsupportedOperatingSystem;
			OutErrorMessage = TEXT("Windows Runtime initialization failed; Windows Graphics Capture is unavailable.");
			return false;
		}
		bNeedsRoUninitialize = RoResult == S_OK || RoResult == S_FALSE;

		ComPtr<IGraphicsCaptureSessionStatics> SessionStatics;
		boolean bSupported = false;
		if (FAILED(GetActivationFactory(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureSession, SessionStatics))
			|| FAILED(SessionStatics->IsSupported(&bSupported)) || !bSupported)
		{
			OutError = EBertaDesktopCaptureError::UnsupportedOperatingSystem;
			OutErrorMessage = TEXT("Windows Graphics Capture is not supported by this Windows installation.");
			Stop();
			return false;
		}

		ComPtr<IGraphicsCaptureItemInterop> ItemInterop;
		if (FAILED(GetActivationFactory(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureItem, ItemInterop)))
		{
			OutError = EBertaDesktopCaptureError::UnsupportedOperatingSystem;
			OutErrorMessage = TEXT("Programmatic Windows Graphics Capture interop is unavailable (Windows 10 version 1903 or later is required).");
			Stop();
			return false;
		}

		HRESULT Result = E_INVALIDARG;
		if (Source.Type == EBertaDesktopCaptureSourceType::Display)
		{
			HMONITOR Monitor = nullptr;
			if (!ResolveDisplay(Source.Id, Monitor))
			{
				OutError = EBertaDesktopCaptureError::SourceUnavailable;
				OutErrorMessage = FString::Printf(TEXT("Display capture source '%s' is no longer available."), *Source.Id);
				Stop();
				return false;
			}
			Result = ItemInterop->CreateForMonitor(Monitor, IID_PPV_ARGS(&CaptureItem));
		}
		else if (Source.Type == EBertaDesktopCaptureSourceType::Window)
		{
			HWND Window = nullptr;
			if (!DecodeWindowId(Source.Id, Window))
			{
				OutError = EBertaDesktopCaptureError::SourceUnavailable;
				OutErrorMessage = TEXT("The window capture source identifier is invalid, stale, or belongs to another runtime session.");
				Stop();
				return false;
			}
			Result = ItemInterop->CreateForWindow(Window, IID_PPV_ARGS(&CaptureItem));
		}
		else
		{
			OutError = EBertaDesktopCaptureError::InvalidSource;
			OutErrorMessage = TEXT("Desktop capture source type is invalid.");
			Stop();
			return false;
		}

		ABI::Windows::Graphics::SizeInt32 InitialSize{};
		if (FAILED(Result) || !CaptureItem || FAILED(CaptureItem->get_Size(&InitialSize))
			|| InitialSize.Width <= 0 || InitialSize.Height <= 0)
		{
			OutError = EBertaDesktopCaptureError::SourceUnavailable;
			OutErrorMessage = TEXT("Windows could not create a capture item for the selected source.");
			Stop();
			return false;
		}

		D3D_FEATURE_LEVEL CreatedFeatureLevel{};
		Result = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&Device,
			&CreatedFeatureLevel,
			&Context);
		if (FAILED(Result))
		{
			OutErrorMessage = TEXT("Failed to create the dedicated D3D11 device used for desktop capture.");
			Stop();
			return false;
		}

		ComPtr<IDXGIDevice> DxgiDevice;
		ComPtr<IInspectable> InspectableDevice;
		if (FAILED(Device.As(&DxgiDevice))
			|| FAILED(CreateDirect3D11DeviceFromDXGIDevice(DxgiDevice.Get(), &InspectableDevice))
			|| FAILED(InspectableDevice.As(&Direct3DDevice)))
		{
			OutErrorMessage = TEXT("Failed to expose the dedicated D3D11 device to Windows Graphics Capture.");
			Stop();
			return false;
		}

		ComPtr<IDirect3D11CaptureFramePoolStatics2> FramePoolStatics;
		if (FAILED(GetActivationFactory(RuntimeClass_Windows_Graphics_Capture_Direct3D11CaptureFramePool, FramePoolStatics)))
		{
			OutError = EBertaDesktopCaptureError::UnsupportedOperatingSystem;
			OutErrorMessage = TEXT("The free-threaded Windows Graphics Capture frame-pool API is unavailable.");
			Stop();
			return false;
		}
		if (FAILED(FramePoolStatics->CreateFreeThreaded(
				Direct3DDevice.Get(),
				ABI::Windows::Graphics::DirectX::DirectXPixelFormat_B8G8R8A8UIntNormalized,
				CaptureBufferCount,
				InitialSize,
				&FramePool))
			|| !FramePool)
		{
			OutErrorMessage = TEXT("Failed to create the free-threaded Windows Graphics Capture frame pool.");
			Stop();
			return false;
		}

		CallbackState = MakeShared<FCaptureCallbackState, ESPMode::ThreadSafe>(
			MaxFrameRate,
			FIntPoint(InitialSize.Width, InitialSize.Height),
			Dispatcher);
		CallbackState->SetResources(Device, Context, Direct3DDevice, FramePool);
		const TWeakPtr<FCaptureCallbackState, ESPMode::ThreadSafe> WeakState = CallbackState;

		FrameArrivedHandler = Callback<FFrameArrivedHandler>(
			[WeakState](ABI::Windows::Graphics::Capture::IDirect3D11CaptureFramePool*, IInspectable*) -> HRESULT
			{
				if (TSharedPtr<FCaptureCallbackState, ESPMode::ThreadSafe> State = WeakState.Pin())
				{
					return State->OnFrameArrived();
				}
				return S_OK;
			});
		ItemClosedHandler = Callback<FItemClosedHandler>(
			[WeakState](ABI::Windows::Graphics::Capture::IGraphicsCaptureItem*, IInspectable*) -> HRESULT
			{
				if (TSharedPtr<FCaptureCallbackState, ESPMode::ThreadSafe> State = WeakState.Pin())
				{
					return State->OnItemClosed();
				}
				return S_OK;
			});

		if (!FrameArrivedHandler || !ItemClosedHandler
			|| FAILED(FramePool->add_FrameArrived(FrameArrivedHandler.Get(), &FrameArrivedToken)))
		{
			OutErrorMessage = TEXT("Failed to register Windows Graphics Capture lifecycle callbacks.");
			Stop();
			return false;
		}
		bFrameArrivedRegistered = true;
		if (FAILED(CaptureItem->add_Closed(ItemClosedHandler.Get(), &ItemClosedToken)))
		{
			OutErrorMessage = TEXT("Failed to register Windows Graphics Capture lifecycle callbacks.");
			Stop();
			return false;
		}
		bItemClosedRegistered = true;

		if (FAILED(FramePool->CreateCaptureSession(CaptureItem.Get(), &CaptureSession))
			|| !CaptureSession || FAILED(CaptureSession->StartCapture()))
		{
			OutErrorMessage = TEXT("Windows Graphics Capture could not start the selected capture session.");
			Stop();
			return false;
		}

		OutInitialSize = FIntPoint(InitialSize.Width, InitialSize.Height);
		OutError = EBertaDesktopCaptureError::None;
		OutErrorMessage.Reset();
		return true;
	}

	void Stop()
	{
		using namespace BertaDesktopCapture::Private;

		if (CallbackState)
		{
			CallbackState->Deactivate();
		}
		if (bFrameArrivedRegistered && FramePool)
		{
			FramePool->remove_FrameArrived(FrameArrivedToken);
		}
		if (bItemClosedRegistered && CaptureItem)
		{
			CaptureItem->remove_Closed(ItemClosedToken);
		}
		bFrameArrivedRegistered = false;
		bItemClosedRegistered = false;

		CloseInspectable(CaptureSession.Get());
		CloseInspectable(FramePool.Get());
		CloseInspectable(CaptureItem.Get());
		CaptureSession.Reset();
		FramePool.Reset();
		CaptureItem.Reset();
		FrameArrivedHandler.Reset();
		ItemClosedHandler.Reset();
		CallbackState.Reset();
		CloseInspectable(Direct3DDevice.Get());
		Direct3DDevice.Reset();
		Context.Reset();
		Device.Reset();

		if (bNeedsRoUninitialize)
		{
			RoUninitialize();
			bNeedsRoUninitialize = false;
		}
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
	Microsoft::WRL::ComPtr<ABI::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice> Direct3DDevice;
	Microsoft::WRL::ComPtr<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem> CaptureItem;
	Microsoft::WRL::ComPtr<ABI::Windows::Graphics::Capture::IDirect3D11CaptureFramePool> FramePool;
	Microsoft::WRL::ComPtr<ABI::Windows::Graphics::Capture::IGraphicsCaptureSession> CaptureSession;
	Microsoft::WRL::ComPtr<BertaDesktopCapture::Private::FFrameArrivedHandler> FrameArrivedHandler;
	Microsoft::WRL::ComPtr<BertaDesktopCapture::Private::FItemClosedHandler> ItemClosedHandler;
	TSharedPtr<BertaDesktopCapture::Private::FCaptureCallbackState, ESPMode::ThreadSafe> CallbackState;
	EventRegistrationToken FrameArrivedToken{};
	EventRegistrationToken ItemClosedToken{};
	bool bFrameArrivedRegistered = false;
	bool bItemClosedRegistered = false;
	bool bNeedsRoUninitialize = false;
};

FBertaWindowsCaptureBackend::FBertaWindowsCaptureBackend()
	: Implementation(MakeUnique<FImplementation>())
{
}

FBertaWindowsCaptureBackend::~FBertaWindowsCaptureBackend() = default;

bool FBertaWindowsCaptureBackend::IsSupported()
{
	return BertaDesktopCapture::Private::CheckRuntimeSupport();
}

bool FBertaWindowsCaptureBackend::EnumerateWindows(
	TArray<FBertaDesktopCaptureSource>& OutSources,
	FString& OutErrorMessage)
{
	OutSources.Reset();
	OutErrorMessage.Reset();
	if (!::EnumWindows(BertaDesktopCapture::Private::EnumerateWindowCallback, reinterpret_cast<LPARAM>(&OutSources)))
	{
		OutErrorMessage = TEXT("Windows failed to enumerate top-level desktop windows.");
		return false;
	}
	return true;
}

bool FBertaWindowsCaptureBackend::Initialize(
	const FBertaDesktopCaptureSource& Source,
	const int32 MaxFrameRate,
	const TSharedRef<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe>& Dispatcher,
	FIntPoint& OutInitialSize,
	EBertaDesktopCaptureError& OutError,
	FString& OutErrorMessage)
{
	return Implementation->Initialize(
		Source,
		MaxFrameRate,
		Dispatcher,
		OutInitialSize,
		OutError,
		OutErrorMessage);
}

void FBertaWindowsCaptureBackend::Stop()
{
	Implementation->Stop();
}
