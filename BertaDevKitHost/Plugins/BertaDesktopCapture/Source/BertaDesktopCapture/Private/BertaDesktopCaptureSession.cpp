#include "BertaDesktopCaptureSession.h"

#include "Async/Async.h"
#include "BertaDesktopCapture.h"
#include "BertaDesktopCaptureDispatcher.h"
#include "BertaDesktopCaptureFrameUtils.h"
#include "BertaDesktopCaptureSubsystem.h"
#include "Engine/Texture2D.h"
#include "RHITypes.h"
#include "Windows/BertaWindowsCaptureBackend.h"

UBertaDesktopCaptureSession::~UBertaDesktopCaptureSession()
{
	if (Dispatcher)
	{
		Dispatcher->Suppress();
	}
	if (NativeBackend)
	{
		NativeBackend->Stop();
	}
}

bool UBertaDesktopCaptureSession::IsCapturing() const
{
	return bIsCapturing;
}

FBertaDesktopCaptureSource UBertaDesktopCaptureSession::GetSource() const
{
	return Source;
}

UTexture2D* UBertaDesktopCaptureSession::GetTexture() const
{
	return Texture;
}

FIntPoint UBertaDesktopCaptureSession::GetFrameSize() const
{
	return FrameSize;
}

bool UBertaDesktopCaptureSession::StopCapture()
{
	if (!IsInGameThread() || !bIsCapturing)
	{
		return false;
	}

	Finish(EBertaDesktopCaptureStopReason::UserStopped, FString(), true);
	return true;
}

void UBertaDesktopCaptureSession::BeginDestroy()
{
	if (bIsCapturing)
	{
		Finish(EBertaDesktopCaptureStopReason::UserStopped, FString(), false);
	}
	Super::BeginDestroy();
}

bool UBertaDesktopCaptureSession::InitializeInternal(
	const FBertaDesktopCaptureSource& InSource,
	const FBertaDesktopCaptureOptions& Options,
	UBertaDesktopCaptureSubsystem* InOwner,
	EBertaDesktopCaptureError& OutError,
	FString& OutErrorMessage)
{
	check(IsInGameThread());
	check(InOwner != nullptr);
	check(!bIsCapturing);

	Source = InSource;
	OwnerSubsystem = InOwner;
	Dispatcher = MakeShared<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe>(this);
	NativeBackend = MakeUnique<FBertaWindowsCaptureBackend>();

	FIntPoint InitialSize = FIntPoint::ZeroValue;
	if (!NativeBackend->Initialize(
		Source,
		Options.MaxFrameRate,
		Dispatcher.ToSharedRef(),
		InitialSize,
		OutError,
		OutErrorMessage))
	{
		Dispatcher->Suppress();
		Dispatcher.Reset();
		NativeBackend.Reset();
		OwnerSubsystem = nullptr;
		return false;
	}

	if (!CreateTexture(InitialSize))
	{
		OutError = EBertaDesktopCaptureError::TextureCreationFailed;
		OutErrorMessage = FString::Printf(
			TEXT("Failed to create a transient %dx%d BGRA8 texture for desktop capture."),
			InitialSize.X,
			InitialSize.Y);
		Dispatcher->Suppress();
		NativeBackend->Stop();
		NativeBackend.Reset();
		Dispatcher.Reset();
		OwnerSubsystem = nullptr;
		return false;
	}

	bIsCapturing = true;
	return true;
}

void UBertaDesktopCaptureSession::HandleCapturedFrame(
	const FIntPoint Size,
	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Pixels)
{
	check(IsInGameThread());
	if (!bIsCapturing || Size != FrameSize || !Pixels.IsValid())
	{
		return;
	}

	int32 ExpectedByteCount = 0;
	if (!BertaDesktopCapture::Private::IsValidFrameSize(Size, ExpectedByteCount)
		|| Pixels->Num() != ExpectedByteCount)
	{
		HandleNativeStopped(
			EBertaDesktopCaptureStopReason::CaptureFailed,
			TEXT("A desktop capture frame did not match its declared BGRA8 dimensions."));
		return;
	}

	check(!bUploadInFlight);
	UploadFrame(Size, MoveTemp(Pixels));
}

void UBertaDesktopCaptureSession::HandleCaptureSizeChanged(const FIntPoint Size)
{
	check(IsInGameThread());
	if (!bIsCapturing || Size == FrameSize)
	{
		return;
	}

	if (!CreateTexture(Size))
	{
		HandleNativeStopped(
			EBertaDesktopCaptureStopReason::CaptureFailed,
			FString::Printf(
				TEXT("Failed to replace the desktop capture texture after the source resized to %dx%d."),
				Size.X,
				Size.Y));
		return;
	}

	OnTextureChanged.Broadcast(this, Texture);
}

void UBertaDesktopCaptureSession::HandleNativeStopped(
	const EBertaDesktopCaptureStopReason Reason,
	const FString& ErrorMessage)
{
	check(IsInGameThread());
	if (bIsCapturing)
	{
		if (Reason == EBertaDesktopCaptureStopReason::CaptureFailed)
		{
			UE_LOG(LogBertaDesktopCapture, Warning, TEXT("Desktop capture stopped after a backend failure: %s"), *ErrorMessage);
		}
		Finish(Reason, ErrorMessage, true);
	}
}

void UBertaDesktopCaptureSession::Finish(
	const EBertaDesktopCaptureStopReason Reason,
	const FString& ErrorMessage,
	const bool bBroadcast)
{
	check(IsInGameThread());
	if (!bIsCapturing)
	{
		return;
	}

	bIsCapturing = false;
	if (Dispatcher)
	{
		Dispatcher->Suppress();
	}
	if (NativeBackend)
	{
		NativeBackend->Stop();
		NativeBackend.Reset();
	}
	Dispatcher.Reset();

	UBertaDesktopCaptureSubsystem* Owner = OwnerSubsystem.Get();
	OwnerSubsystem.Reset();
	if (bBroadcast)
	{
		OnStopped.Broadcast(this, Reason, ErrorMessage);
	}
	if (Owner != nullptr)
	{
		Owner->NotifySessionStopped(this);
	}
}

void UBertaDesktopCaptureSession::ShutdownForOwner()
{
	check(IsInGameThread());
	if (bIsCapturing)
	{
		Finish(EBertaDesktopCaptureStopReason::UserStopped, FString(), false);
	}
}

bool UBertaDesktopCaptureSession::CreateTexture(const FIntPoint Size)
{
	check(IsInGameThread());
	int32 ByteCount = 0;
	if (!BertaDesktopCapture::Private::IsValidFrameSize(Size, ByteCount))
	{
		return false;
	}

	TArray64<uint8> InitialPixels;
	InitialPixels.SetNumZeroed(ByteCount);
	UTexture2D* NewTexture = UTexture2D::CreateTransient(
		Size.X,
		Size.Y,
		PF_B8G8R8A8,
		NAME_None,
		InitialPixels);
	if (NewTexture == nullptr)
	{
		return false;
	}

	NewTexture->NeverStream = true;
	NewTexture->SRGB = true;
	Texture = NewTexture;
	FrameSize = Size;
	return true;
}

void UBertaDesktopCaptureSession::UploadFrame(
	const FIntPoint Size,
	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Pixels)
{
	check(IsInGameThread());
	if (Texture == nullptr || !Pixels.IsValid())
	{
		return;
	}
	if (Texture->GetResource() == nullptr)
	{
		HandleNativeStopped(
			EBertaDesktopCaptureStopReason::CaptureFailed,
			TEXT("The desktop capture texture resource was unavailable for an upload."));
		return;
	}

	bUploadInFlight = true;
	if (Dispatcher)
	{
		Dispatcher->SetUploadBlocked(true);
	}
	const TWeakObjectPtr<UBertaDesktopCaptureSession> WeakThis(this);
	TSharedRef<FUpdateTextureRegion2D, ESPMode::ThreadSafe> Region =
		MakeShared<FUpdateTextureRegion2D, ESPMode::ThreadSafe>(
			0,
			0,
			0,
			0,
			static_cast<uint32>(Size.X),
			static_cast<uint32>(Size.Y));
	Texture->UpdateTextureRegions(
		0,
		1,
		&Region.Get(),
		static_cast<uint32>(Size.X) * 4u,
		4,
		Pixels->GetData(),
		[Pixels = MoveTemp(Pixels), Region, WeakThis](uint8*, const FUpdateTextureRegion2D*) mutable
		{
			Pixels.Reset();
			AsyncTask(ENamedThreads::GameThread, [WeakThis]()
			{
				if (UBertaDesktopCaptureSession* Session = WeakThis.Get())
				{
					Session->HandleUploadFinished();
				}
			});
		});
}

void UBertaDesktopCaptureSession::HandleUploadFinished()
{
	check(IsInGameThread());
	bUploadInFlight = false;
	if (Dispatcher)
	{
		Dispatcher->SetUploadBlocked(false);
	}
}
