#include "Windows/BertaWindowsSerialEnumeration.h"

#include "BertaSerialUtils.h"
#include "Windows/BertaWindowsSerialCommon.h"

#include "Windows/WindowsHWrapper.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <SetupAPI.h>
#include <ntddser.h>
#include "Windows/HideWindowsPlatformTypes.h"

namespace
{
	class FScopedDeviceInfoSet final
	{
	public:
		explicit FScopedDeviceInfoSet(const HDEVINFO InHandle)
			: Handle(InHandle)
		{
		}

		~FScopedDeviceInfoSet()
		{
			if (Handle != INVALID_HANDLE_VALUE)
			{
				::SetupDiDestroyDeviceInfoList(Handle);
			}
		}

		HDEVINFO Get() const
		{
			return Handle;
		}

	private:
		HDEVINFO Handle = INVALID_HANDLE_VALUE;
	};

	bool ReadDeviceProperty(
		const HDEVINFO DeviceInfoSet,
		SP_DEVINFO_DATA& DeviceInfo,
		const DWORD Property,
		TArray<uint8>& OutBytes,
		DWORD& OutType)
	{
		OutBytes.Reset();
		OutType = REG_NONE;
		DWORD RequiredSize = 0;
		::SetupDiGetDeviceRegistryPropertyW(
			DeviceInfoSet,
			&DeviceInfo,
			Property,
			&OutType,
			nullptr,
			0,
			&RequiredSize);
		if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER || RequiredSize == 0)
		{
			return false;
		}

		OutBytes.SetNumZeroed(static_cast<int32>(RequiredSize + sizeof(WCHAR)));
		return ::SetupDiGetDeviceRegistryPropertyW(
			DeviceInfoSet,
			&DeviceInfo,
			Property,
			&OutType,
			OutBytes.GetData(),
			RequiredSize,
			&RequiredSize) != 0;
	}

	FString ReadStringProperty(
		const HDEVINFO DeviceInfoSet,
		SP_DEVINFO_DATA& DeviceInfo,
		const DWORD Property)
	{
		TArray<uint8> Bytes;
		DWORD Type = REG_NONE;
		if (!ReadDeviceProperty(DeviceInfoSet, DeviceInfo, Property, Bytes, Type)
			|| (Type != REG_SZ && Type != REG_EXPAND_SZ))
		{
			return FString();
		}
		return FString(reinterpret_cast<const WCHAR*>(Bytes.GetData()));
	}

	TArray<FString> ReadMultiStringProperty(
		const HDEVINFO DeviceInfoSet,
		SP_DEVINFO_DATA& DeviceInfo,
		const DWORD Property)
	{
		TArray<uint8> Bytes;
		DWORD Type = REG_NONE;
		if (!ReadDeviceProperty(DeviceInfoSet, DeviceInfo, Property, Bytes, Type) || Type != REG_MULTI_SZ)
		{
			return {};
		}

		TArray<FString> Values;
		const WCHAR* Cursor = reinterpret_cast<const WCHAR*>(Bytes.GetData());
		while (*Cursor != L'\0')
		{
			const int32 Length = FCStringWide::Strlen(Cursor);
			Values.Emplace(Length, Cursor);
			Cursor += Length + 1;
		}
		return Values;
	}

	FString ReadPortName(const HDEVINFO DeviceInfoSet, SP_DEVINFO_DATA& DeviceInfo)
	{
		HKEY Key = ::SetupDiOpenDevRegKey(
			DeviceInfoSet,
			&DeviceInfo,
			DICS_FLAG_GLOBAL,
			0,
			DIREG_DEV,
			KEY_QUERY_VALUE);
		if (Key == INVALID_HANDLE_VALUE)
		{
			return FString();
		}

		DWORD Type = REG_NONE;
		DWORD ByteCount = 0;
		const LONG SizeResult = ::RegQueryValueExW(Key, L"PortName", nullptr, &Type, nullptr, &ByteCount);
		FString Result;
		if (SizeResult == ERROR_SUCCESS && Type == REG_SZ && ByteCount > sizeof(WCHAR))
		{
			TArray<WCHAR> Buffer;
			Buffer.SetNumZeroed(static_cast<int32>(ByteCount / sizeof(WCHAR) + 1));
			if (::RegQueryValueExW(
					Key,
					L"PortName",
					nullptr,
					&Type,
					reinterpret_cast<BYTE*>(Buffer.GetData()),
					&ByteCount) == ERROR_SUCCESS)
			{
				Result = Buffer.GetData();
			}
		}
		::RegCloseKey(Key);
		return Result;
	}

	FString ReadDeviceInstanceId(const HDEVINFO DeviceInfoSet, SP_DEVINFO_DATA& DeviceInfo)
	{
		DWORD RequiredCharacters = 0;
		::SetupDiGetDeviceInstanceIdW(DeviceInfoSet, &DeviceInfo, nullptr, 0, &RequiredCharacters);
		if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER || RequiredCharacters == 0)
		{
			return FString();
		}
		TArray<WCHAR> Buffer;
		Buffer.SetNumZeroed(static_cast<int32>(RequiredCharacters + 1));
		if (!::SetupDiGetDeviceInstanceIdW(
				DeviceInfoSet,
				&DeviceInfo,
				Buffer.GetData(),
				RequiredCharacters,
				&RequiredCharacters))
		{
			return FString();
		}
		return FString(Buffer.GetData());
	}
}

bool FBertaWindowsSerialEnumeration::GetPorts(
	TArray<FBertaSerialPortInfo>& OutPorts,
	FString& OutErrorMessage)
{
	OutPorts.Reset();
	OutErrorMessage.Reset();

	FScopedDeviceInfoSet DeviceInfoSet(::SetupDiGetClassDevsW(
		&GUID_DEVINTERFACE_COMPORT,
		nullptr,
		nullptr,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
	if (DeviceInfoSet.Get() == INVALID_HANDLE_VALUE)
	{
		const uint32 ErrorCode = ::GetLastError();
		OutErrorMessage = FString::Printf(
			TEXT("Windows could not enumerate present COM-port interfaces: %s."),
			*BertaSerial::Windows::FormatWindowsError(ErrorCode));
		return false;
	}

	TSet<FString> SeenPortNames;
	for (DWORD InterfaceIndex = 0;; ++InterfaceIndex)
	{
		SP_DEVICE_INTERFACE_DATA InterfaceData{};
		InterfaceData.cbSize = sizeof(InterfaceData);
		if (!::SetupDiEnumDeviceInterfaces(
				DeviceInfoSet.Get(),
				nullptr,
				&GUID_DEVINTERFACE_COMPORT,
				InterfaceIndex,
				&InterfaceData))
		{
			const uint32 ErrorCode = ::GetLastError();
			if (ErrorCode == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
			OutPorts.Reset();
			OutErrorMessage = FString::Printf(
				TEXT("Windows stopped enumerating COM-port interfaces: %s."),
				*BertaSerial::Windows::FormatWindowsError(ErrorCode));
			return false;
		}

		DWORD DetailSize = 0;
		::SetupDiGetDeviceInterfaceDetailW(
			DeviceInfoSet.Get(),
			&InterfaceData,
			nullptr,
			0,
			&DetailSize,
			nullptr);
		if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER || DetailSize == 0)
		{
			continue;
		}

		TArray<uint8> DetailStorage;
		DetailStorage.SetNumZeroed(static_cast<int32>(DetailSize));
		auto* Detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(DetailStorage.GetData());
		Detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
		SP_DEVINFO_DATA DeviceInfo{};
		DeviceInfo.cbSize = sizeof(DeviceInfo);
		if (!::SetupDiGetDeviceInterfaceDetailW(
				DeviceInfoSet.Get(),
				&InterfaceData,
				Detail,
				DetailSize,
				nullptr,
				&DeviceInfo))
		{
			continue;
		}

		FString NormalizedPortName;
		if (!BertaSerial::Private::NormalizePortName(
				ReadPortName(DeviceInfoSet.Get(), DeviceInfo),
				NormalizedPortName)
			|| SeenPortNames.Contains(NormalizedPortName))
		{
			continue;
		}
		SeenPortNames.Add(NormalizedPortName);

		FBertaSerialPortInfo& Port = OutPorts.AddDefaulted_GetRef();
		Port.PortName = NormalizedPortName;
		Port.FriendlyName = ReadStringProperty(DeviceInfoSet.Get(), DeviceInfo, SPDRP_FRIENDLYNAME);
		if (Port.FriendlyName.IsEmpty())
		{
			Port.FriendlyName = ReadStringProperty(DeviceInfoSet.Get(), DeviceInfo, SPDRP_DEVICEDESC);
		}
		if (Port.FriendlyName.IsEmpty())
		{
			Port.FriendlyName = Port.PortName;
		}
		Port.Manufacturer = ReadStringProperty(DeviceInfoSet.Get(), DeviceInfo, SPDRP_MFG);
		Port.DeviceInstanceId = ReadDeviceInstanceId(DeviceInfoSet.Get(), DeviceInfo);
		Port.HardwareIds = ReadMultiStringProperty(DeviceInfoSet.Get(), DeviceInfo, SPDRP_HARDWAREID);
	}

	OutPorts.Sort([](const FBertaSerialPortInfo& Left, const FBertaSerialPortInfo& Right)
	{
		return BertaSerial::Private::PortNameLess(Left.PortName, Right.PortName);
	});
	return true;
}
