#pragma once

#include "BertaProcessTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "BertaProcessSubsystem.generated.h"

class UBertaProcess;

/** Owns external processes for exactly one GameInstance lifetime. */
UCLASS()
class BERTAPROCESSBRIDGE_API UBertaProcessSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	bool LaunchProcess(
		const FBertaProcessLaunchOptions& Options,
		UBertaProcess*& OutProcess,
		EBertaProcessLaunchError& OutError,
		FString& OutErrorMessage);

private:
	friend class UBertaProcess;

	void NotifyProcessFinished(UBertaProcess* Process);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBertaProcess>> ActiveProcesses;

	bool bIsShuttingDown = false;
};
