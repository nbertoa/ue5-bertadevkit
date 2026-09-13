#pragma once

#include "BertaProcessTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaProcessBlueprintLibrary.generated.h"

class UBertaProcess;

UCLASS()
class BERTAPROCESSBRIDGE_API UBertaProcessBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "BertaProcessBridge|Process",
		meta = (
			WorldContext = "WorldContextObject",
			DisplayName = "Launch External Process",
			ReturnDisplayName = "Success"))
	static bool LaunchProcess(
		const UObject* WorldContextObject,
		const FBertaProcessLaunchOptions& Options,
		UBertaProcess*& OutProcess,
		EBertaProcessLaunchError& OutError,
		FString& OutErrorMessage);
};
