#include "BertaDualSenseBlueprintLibrary.h"

#include "BertaDualSense.h"
#include "Modules/ModuleManager.h"

bool UBertaDualSenseBlueprintLibrary::SetMicrophoneLed(int32 ControllerId, bool bEnabled)
{
	if (FBertaDualSenseModule* Module = FModuleManager::GetModulePtr<FBertaDualSenseModule>(TEXT("BertaDualSense")))
	{
		Module->SetMicrophoneLed(ControllerId, bEnabled);
		return true;
	}

	return false;
}
