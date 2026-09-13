#pragma once

#include "BertaDesktopCaptureTypes.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"

#include "BertaDesktopCaptureSession.generated.h"

class FBertaDesktopCaptureDispatcher;
class FBertaWindowsCaptureBackend;
class UBertaDesktopCaptureSession;
class UBertaDesktopCaptureSubsystem;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBertaDesktopCaptureTextureChanged,
	UBertaDesktopCaptureSession*, Session,
	UTexture2D*, Texture);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FBertaDesktopCaptureStopped,
	UBertaDesktopCaptureSession*, Session,
	EBertaDesktopCaptureStopReason, Reason,
	const FString&, ErrorMessage);

/** One display or window capture owned by a GameInstance. */
UCLASS(BlueprintType)
class BERTADESKTOPCAPTURE_API UBertaDesktopCaptureSession : public UObject
{
	GENERATED_BODY()

public:
	virtual ~UBertaDesktopCaptureSession() override;

	UPROPERTY(BlueprintAssignable, Category = "BertaDesktopCapture|Capture")
	FBertaDesktopCaptureTextureChanged OnTextureChanged;

	UPROPERTY(BlueprintAssignable, Category = "BertaDesktopCapture|Capture")
	FBertaDesktopCaptureStopped OnStopped;

	UFUNCTION(BlueprintPure, Category = "BertaDesktopCapture|Capture")
	bool IsCapturing() const;

	UFUNCTION(BlueprintPure, Category = "BertaDesktopCapture|Capture")
	FBertaDesktopCaptureSource GetSource() const;

	UFUNCTION(BlueprintPure, Category = "BertaDesktopCapture|Capture")
	UTexture2D* GetTexture() const;

	UFUNCTION(BlueprintPure, Category = "BertaDesktopCapture|Capture")
	FIntPoint GetFrameSize() const;

	UFUNCTION(BlueprintCallable, Category = "BertaDesktopCapture|Capture")
	bool StopCapture();

	virtual void BeginDestroy() override;

private:
	friend class FBertaDesktopCaptureDispatcher;
	friend class UBertaDesktopCaptureSubsystem;

	bool InitializeInternal(
		const FBertaDesktopCaptureSource& InSource,
		const FBertaDesktopCaptureOptions& Options,
		UBertaDesktopCaptureSubsystem* InOwner,
		EBertaDesktopCaptureError& OutError,
		FString& OutErrorMessage);
	void HandleCapturedFrame(FIntPoint Size, TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Pixels);
	void HandleCaptureSizeChanged(FIntPoint Size);
	void HandleNativeStopped(EBertaDesktopCaptureStopReason Reason, const FString& ErrorMessage);
	void Finish(EBertaDesktopCaptureStopReason Reason, const FString& ErrorMessage, bool bBroadcast);
	void ShutdownForOwner();
	bool CreateTexture(FIntPoint Size);
	void UploadFrame(FIntPoint Size, TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Pixels);
	void HandleUploadFinished();

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> Texture;

	UPROPERTY()
	FBertaDesktopCaptureSource Source;

	FIntPoint FrameSize = FIntPoint::ZeroValue;
	TWeakObjectPtr<UBertaDesktopCaptureSubsystem> OwnerSubsystem;
	TSharedPtr<FBertaDesktopCaptureDispatcher, ESPMode::ThreadSafe> Dispatcher;
	TUniquePtr<FBertaWindowsCaptureBackend> NativeBackend;
	bool bUploadInFlight = false;
	bool bIsCapturing = false;
};
