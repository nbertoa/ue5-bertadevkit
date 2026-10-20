#pragma once

#include "Containers/UnrealString.h"

class UClass;
class UBertaUGCCameraCycleComponent;
class UUGC_CameraDataAssetBase;

namespace BertaUGCAudit
{
	void Run();

	// Inspect compiled defaults/templates only; never execute camera initialization.
	bool ResolveDefaultCameraData(const UClass* ManagerClass, const UUGC_CameraDataAssetBase*& OutData, FString& OutReason);
	const UBertaUGCCameraCycleComponent* ResolveCycleTemplate(const UClass* ControllerClass, FString& OutReason);
}
