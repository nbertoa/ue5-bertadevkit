#pragma once

#include "CoreMinimal.h"
#include "BertaSystemInfoTypes.generated.h"

USTRUCT(BlueprintType)
struct BERTASYSTEMINFO_API FBertaOperatingSystemInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString PlatformName;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString Version;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString VersionLabel;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString SubVersionLabel;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString Architecture;
};

USTRUCT(BlueprintType)
struct BERTASYSTEMINFO_API FBertaCPUInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString Brand;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString Vendor;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int32 PhysicalCoreCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int32 LogicalCoreCount = 0;
};

USTRUCT(BlueprintType)
struct BERTASYSTEMINFO_API FBertaGPUInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString AdapterName;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString ProviderName;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString RHIName;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString DriverVersion;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	FString DriverDate;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int32 VendorId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int32 DeviceId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 DedicatedVideoMemoryBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	bool bHasRHIInfo = false;
};

USTRUCT(BlueprintType)
struct BERTASYSTEMINFO_API FBertaMemoryInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 TotalPhysicalBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 AvailablePhysicalBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 TotalVirtualBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 AvailableVirtualBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 ProcessUsedPhysicalBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 ProcessPeakUsedPhysicalBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 ProcessUsedVirtualBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|System")
	int64 ProcessPeakUsedVirtualBytes = 0;
};

USTRUCT(BlueprintType)
struct BERTASYSTEMINFO_API FBertaDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FString FriendlyName;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	bool bIsPrimary = false;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FIntPoint DesktopPosition = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FIntPoint Resolution = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FIntPoint WorkAreaPosition = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FIntPoint WorkAreaSize = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	int32 DPI = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FIntPoint NativeResolution = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FIntPoint MaxResolution = FIntPoint::ZeroValue;
};

USTRUCT(BlueprintType)
struct BERTASYSTEMINFO_API FBertaDisplayMode
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	FIntPoint Resolution = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Displays")
	int32 RefreshRateHz = 0;
};

USTRUCT(BlueprintType)
struct BERTASYSTEMINFO_API FBertaAudioInputDeviceInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	FString DeviceId;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	int32 InputChannels = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	int32 PreferredSampleRate = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	bool bSupportsHardwareAEC = false;
};

USTRUCT(BlueprintType)
struct BERTASYSTEMINFO_API FBertaAudioOutputDeviceInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	FString DeviceId;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	int32 NumChannels = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	int32 SampleRate = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	bool bIsSystemDefault = false;

	UPROPERTY(BlueprintReadOnly, Category = "BertaSystemInfo|Audio")
	bool bIsCurrentDevice = false;
};
