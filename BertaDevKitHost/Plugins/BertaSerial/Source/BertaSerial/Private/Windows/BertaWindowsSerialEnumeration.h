#pragma once

#include "BertaSerialTypes.h"

class FBertaWindowsSerialEnumeration final
{
public:
	static bool GetPorts(TArray<FBertaSerialPortInfo>& OutPorts, FString& OutErrorMessage);
};
