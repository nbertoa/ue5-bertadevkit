#include "BertaSystemInfoBlueprintLibrary.h"

#include "AudioCaptureCore.h"
#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "AudioMixer.h"
#include "AudioMixerDevice.h"
#include "AudioThread.h"
#include "DynamicRHI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericApplication.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProperties.h"
#include "RHIGlobals.h"

namespace
{
	int64 ToBlueprintInt64(const uint64 Value)
	{
		return Value > static_cast<uint64>(MAX_int64) ? MAX_int64 : static_cast<int64>(Value);
	}

	int32 ToBlueprintInt32(const uint32 Value)
	{
		return Value > static_cast<uint32>(MAX_int32) ? MAX_int32 : static_cast<int32>(Value);
	}

	FBertaDisplayInfo BuildDisplayInfo(const FMonitorInfo& Monitor)
	{
		FBertaDisplayInfo Info;
		Info.Id = Monitor.ID;
		Info.Name = Monitor.Name;
		Info.FriendlyName = Monitor.FriendlyName;
		Info.bIsPrimary = Monitor.bIsPrimary;
		Info.DesktopPosition = FIntPoint(Monitor.DisplayRect.Left, Monitor.DisplayRect.Top);
		Info.Resolution = FIntPoint(
			Monitor.DisplayRect.Right - Monitor.DisplayRect.Left,
			Monitor.DisplayRect.Bottom - Monitor.DisplayRect.Top);
		Info.WorkAreaPosition = FIntPoint(Monitor.WorkArea.Left, Monitor.WorkArea.Top);
		Info.WorkAreaSize = FIntPoint(
			Monitor.WorkArea.Right - Monitor.WorkArea.Left,
			Monitor.WorkArea.Bottom - Monitor.WorkArea.Top);
		Info.DPI = Monitor.DPI;
		Info.NativeResolution = FIntPoint(Monitor.NativeWidth, Monitor.NativeHeight);
		Info.MaxResolution = Monitor.MaxResolution;
		return Info;
	}

	template <typename QueryType>
	bool RunAudioQuerySynchronously(QueryType&& Query)
	{
		if (IsInAudioThread())
		{
			Query();
			return true;
		}

		if (!IsInGameThread())
		{
			return false;
		}

		FAudioThread::RunCommandOnAudioThread(Forward<QueryType>(Query));
		FAudioCommandFence Fence;
		Fence.BeginFence();
		Fence.Wait();
		return true;
	}

	bool HasUsableDeviceId(const FString& DeviceId)
	{
		return !DeviceId.IsEmpty() && !DeviceId.Equals(TEXT("Unknown"), ESearchCase::IgnoreCase);
	}

	bool HasUsableDeviceName(const FString& DeviceName)
	{
		return !DeviceName.IsEmpty() && !DeviceName.Equals(TEXT("Unknown"), ESearchCase::IgnoreCase);
	}

	bool IsCurrentAudioDevice(const Audio::FAudioPlatformDeviceInfo& Device, const Audio::FAudioPlatformDeviceInfo& CurrentDevice)
	{
		if (HasUsableDeviceId(Device.DeviceId) && HasUsableDeviceId(CurrentDevice.DeviceId))
		{
			return Device.DeviceId == CurrentDevice.DeviceId;
		}

		return HasUsableDeviceName(Device.Name)
			&& HasUsableDeviceName(CurrentDevice.Name)
			&& Device.Name == CurrentDevice.Name;
	}

	bool EnumerateAudioOutputDevices(
		Audio::FMixerDevice& MixerDevice,
		TArray<FBertaAudioOutputDeviceInfo>& OutDevices)
	{
		Audio::IAudioMixerPlatformInterface* MixerPlatform = MixerDevice.GetAudioMixerPlatform();
		if (!MixerPlatform || !MixerPlatform->IsInitialized())
		{
			return false;
		}

		uint32 NumDevices = 0;
		if (!MixerPlatform->GetNumOutputDevices(NumDevices))
		{
			return false;
		}

		uint32 DefaultDeviceIndex = 0;
		const bool bHasDefaultDeviceIndex = MixerPlatform->GetDefaultOutputDeviceIndex(DefaultDeviceIndex);
		const Audio::FAudioPlatformDeviceInfo CurrentDevice = MixerPlatform->GetPlatformDeviceInfo();

		TArray<FBertaAudioOutputDeviceInfo> Devices;
		Devices.Reserve(ToBlueprintInt32(NumDevices));
		for (uint32 DeviceIndex = 0; DeviceIndex < NumDevices; ++DeviceIndex)
		{
			Audio::FAudioPlatformDeviceInfo Device;
			if (!MixerPlatform->GetOutputDeviceInfo(DeviceIndex, Device))
			{
				return false;
			}

			FBertaAudioOutputDeviceInfo Info;
			Info.DeviceId = Device.DeviceId;
			Info.Name = Device.Name;
			Info.NumChannels = Device.NumChannels;
			Info.SampleRate = Device.SampleRate;
			Info.bIsSystemDefault = Device.bIsSystemDefault || (bHasDefaultDeviceIndex && DeviceIndex == DefaultDeviceIndex);
			Info.bIsCurrentDevice = IsCurrentAudioDevice(Device, CurrentDevice);
			Devices.Add(MoveTemp(Info));
		}

		OutDevices = MoveTemp(Devices);
		return true;
	}
}

FBertaOperatingSystemInfo UBertaSystemInfoBlueprintLibrary::GetOperatingSystemInfo()
{
	FBertaOperatingSystemInfo Info;
	Info.PlatformName = ANSI_TO_TCHAR(FPlatformProperties::PlatformName());
	Info.Version = FPlatformMisc::GetOSVersion();
	FPlatformMisc::GetOSVersions(Info.VersionLabel, Info.SubVersionLabel);
	Info.Architecture = FPlatformMisc::GetHostArchitecture();
	return Info;
}

FBertaCPUInfo UBertaSystemInfoBlueprintLibrary::GetCPUInfo()
{
	FBertaCPUInfo Info;
	Info.Brand = FPlatformMisc::GetCPUBrand();
	Info.Vendor = FPlatformMisc::GetCPUVendor();
	Info.PhysicalCoreCount = FPlatformMisc::NumberOfCores();
	Info.LogicalCoreCount = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
	return Info;
}

FBertaGPUInfo UBertaSystemInfoBlueprintLibrary::GetGPUInfo()
{
	FBertaGPUInfo Info;
	Info.AdapterName = FPlatformMisc::GetPrimaryGPUBrand();
	const bool bIsRHIInitialized = GDynamicRHI != nullptr && GRHIGlobals.IsRHIInitialized;
	if (bIsRHIInitialized)
	{
		const FRHIGlobals::FGpuInfo& RHIGPUInfo = GRHIGlobals.GpuInfo;
		Info.bHasRHIInfo = !RHIGPUInfo.AdapterName.IsEmpty()
			|| !RHIGPUInfo.AdapterInternalDriverVersion.IsEmpty()
			|| !RHIGPUInfo.AdapterUserDriverVersion.IsEmpty()
			|| !RHIGPUInfo.AdapterDriverDate.IsEmpty()
			|| RHIGPUInfo.VendorId != 0
			|| RHIGPUInfo.DeviceId != 0
			|| RHIGPUInfo.DedicatedVideoMemory != 0;
	}

	if (bIsRHIInitialized)
	{
		const FRHIGlobals::FGpuInfo& RHIGPUInfo = GRHIGlobals.GpuInfo;
		if (!RHIGPUInfo.AdapterName.IsEmpty())
		{
			Info.AdapterName = RHIGPUInfo.AdapterName;
		}
		Info.RHIName = GDynamicRHI->GetName();
		Info.DriverVersion = RHIGPUInfo.AdapterUserDriverVersion;
		if (Info.DriverVersion.IsEmpty())
		{
			Info.DriverVersion = RHIGPUInfo.AdapterInternalDriverVersion;
		}
		Info.DriverDate = RHIGPUInfo.AdapterDriverDate;
		Info.VendorId = ToBlueprintInt32(RHIGPUInfo.VendorId);
		Info.DeviceId = ToBlueprintInt32(RHIGPUInfo.DeviceId);
		Info.DedicatedVideoMemoryBytes = ToBlueprintInt64(RHIGPUInfo.DedicatedVideoMemory);
	}

#if PLATFORM_WINDOWS || PLATFORM_MAC
	const FGPUDriverInfo DriverInfo = FPlatformMisc::GetGPUDriverInfo(Info.AdapterName, false);
#else
	const FGPUDriverInfo DriverInfo = FPlatformMisc::GetGPUDriverInfo(Info.AdapterName);
#endif
	Info.ProviderName = DriverInfo.ProviderName;
	if (Info.AdapterName.IsEmpty())
	{
		Info.AdapterName = DriverInfo.DeviceDescription;
	}
	if (Info.RHIName.IsEmpty())
	{
		Info.RHIName = DriverInfo.RHIName;
	}
	if (Info.DriverVersion.IsEmpty())
	{
		Info.DriverVersion = !DriverInfo.UserDriverVersion.IsEmpty()
			? DriverInfo.UserDriverVersion
			: DriverInfo.InternalDriverVersion;
	}
	if (Info.DriverDate.IsEmpty())
	{
		Info.DriverDate = DriverInfo.DriverDate;
	}
	return Info;
}

FBertaMemoryInfo UBertaSystemInfoBlueprintLibrary::GetMemoryInfo()
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	FBertaMemoryInfo Info;
	Info.TotalPhysicalBytes = ToBlueprintInt64(Stats.TotalPhysical);
	Info.AvailablePhysicalBytes = ToBlueprintInt64(Stats.AvailablePhysical);
	Info.TotalVirtualBytes = ToBlueprintInt64(Stats.TotalVirtual);
	Info.AvailableVirtualBytes = ToBlueprintInt64(Stats.AvailableVirtual);
	Info.ProcessUsedPhysicalBytes = ToBlueprintInt64(Stats.UsedPhysical);
	Info.ProcessPeakUsedPhysicalBytes = ToBlueprintInt64(Stats.PeakUsedPhysical);
	Info.ProcessUsedVirtualBytes = ToBlueprintInt64(Stats.UsedVirtual);
	Info.ProcessPeakUsedVirtualBytes = ToBlueprintInt64(Stats.PeakUsedVirtual);
	return Info;
}

TArray<FBertaDisplayInfo> UBertaSystemInfoBlueprintLibrary::GetDisplays()
{
	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);

	TArray<FBertaDisplayInfo> Displays;
	Displays.Reserve(DisplayMetrics.MonitorInfo.Num());
	for (const FMonitorInfo& Monitor : DisplayMetrics.MonitorInfo)
	{
		Displays.Add(BuildDisplayInfo(Monitor));
	}
	return Displays;
}

bool UBertaSystemInfoBlueprintLibrary::GetPrimaryDisplay(FBertaDisplayInfo& OutDisplay)
{
	OutDisplay = {};
	for (const FBertaDisplayInfo& Display : GetDisplays())
	{
		if (Display.bIsPrimary)
		{
			OutDisplay = Display;
			return true;
		}
	}
	return false;
}

bool UBertaSystemInfoBlueprintLibrary::GetAvailableDisplayModes(
	const FString& DisplayId,
	TArray<FBertaDisplayMode>& OutModes)
{
	OutModes.Reset();

	FDisplayMetrics DisplayMetrics;
	FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
	const FMonitorInfo* Monitor = DisplayMetrics.MonitorInfo.FindByPredicate(
		[&DisplayId](const FMonitorInfo& Candidate)
		{
			return Candidate.ID == DisplayId;
		});
	if (!Monitor || !Monitor->NativeHandle || !GDynamicRHI || !GRHIGlobals.IsRHIInitialized)
	{
		return false;
	}

	const ERHIInterfaceType RHIType = GDynamicRHI->GetInterfaceType();
	if (RHIType != ERHIInterfaceType::D3D11 && RHIType != ERHIInterfaceType::D3D12)
	{
		return false;
	}

	FScreenResolutionArray Resolutions;
	if (!RHIGetAvailableResolutionsForDisplay(Resolutions, false, Monitor->NativeHandle))
	{
		return false;
	}

	TSet<FIntVector> UniqueModes;
	for (const FScreenResolutionRHI& Resolution : Resolutions)
	{
		if (Resolution.Width == 0 || Resolution.Height == 0 || Resolution.RefreshRate == 0
			|| Resolution.Width > static_cast<uint32>(MAX_int32)
			|| Resolution.Height > static_cast<uint32>(MAX_int32)
			|| Resolution.RefreshRate > static_cast<uint32>(MAX_int32))
		{
			continue;
		}

		const FIntVector Key(
			static_cast<int32>(Resolution.Width),
			static_cast<int32>(Resolution.Height),
			static_cast<int32>(Resolution.RefreshRate));
		if (UniqueModes.Contains(Key))
		{
			continue;
		}

		UniqueModes.Add(Key);
		FBertaDisplayMode& Mode = OutModes.AddDefaulted_GetRef();
		Mode.Resolution = FIntPoint(Key.X, Key.Y);
		Mode.RefreshRateHz = Key.Z;
	}

	OutModes.Sort([](const FBertaDisplayMode& Left, const FBertaDisplayMode& Right)
	{
		if (Left.Resolution.X != Right.Resolution.X)
		{
			return Left.Resolution.X < Right.Resolution.X;
		}
		if (Left.Resolution.Y != Right.Resolution.Y)
		{
			return Left.Resolution.Y < Right.Resolution.Y;
		}
		return Left.RefreshRateHz < Right.RefreshRateHz;
	});
	return true;
}

TArray<FBertaAudioInputDeviceInfo> UBertaSystemInfoBlueprintLibrary::GetAudioInputDevices()
{
	TArray<FBertaAudioInputDeviceInfo> Devices;
	RunAudioQuerySynchronously([&Devices]()
	{
		Audio::FAudioCapture AudioCapture;
		TArray<Audio::FCaptureDeviceInfo> CaptureDevices;
		AudioCapture.GetCaptureDevicesAvailable(CaptureDevices);
		Devices.Reserve(CaptureDevices.Num());
		for (const Audio::FCaptureDeviceInfo& CaptureDevice : CaptureDevices)
		{
			FBertaAudioInputDeviceInfo Info;
			Info.DeviceId = CaptureDevice.DeviceId;
			Info.Name = CaptureDevice.DeviceName;
			Info.InputChannels = CaptureDevice.InputChannels;
			Info.PreferredSampleRate = CaptureDevice.PreferredSampleRate;
			Info.bSupportsHardwareAEC = CaptureDevice.bSupportsHardwareAEC;
			Devices.Add(MoveTemp(Info));
		}
	});
	return Devices;
}

bool UBertaSystemInfoBlueprintLibrary::GetAudioOutputDevices(
	const UObject* WorldContextObject,
	TArray<FBertaAudioOutputDeviceInfo>& OutDevices)
{
	OutDevices.Reset();
	if (!WorldContextObject || !GEngine || (!IsInGameThread() && !IsInAudioThread()))
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World || !World->bAllowAudioPlayback || World->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

	FAudioDeviceManager* AudioDeviceManager = GEngine->GetAudioDeviceManager();
	IAudioDeviceModule* AudioDeviceModule = AudioDeviceManager ? AudioDeviceManager->GetAudioDeviceModule() : nullptr;
	if (!AudioDeviceModule || !AudioDeviceModule->IsAudioMixerModule())
	{
		return false;
	}

	FAudioDeviceHandle AudioDeviceHandle = World->GetAudioDevice();
	if (!AudioDeviceHandle)
	{
		return false;
	}

	bool bQuerySucceeded = false;
	const bool bRanQuery = RunAudioQuerySynchronously(
		[AudioDeviceHandle, &OutDevices, &bQuerySucceeded]() mutable
		{
			FAudioDevice* AudioDevice = AudioDeviceHandle.GetAudioDevice();
			if (!AudioDevice)
			{
				return;
			}

			bQuerySucceeded = EnumerateAudioOutputDevices(
				*static_cast<Audio::FMixerDevice*>(AudioDevice),
				OutDevices);
		});

	if (!bRanQuery || !bQuerySucceeded)
	{
		OutDevices.Reset();
		return false;
	}
	return true;
}
