#pragma once

#include "CoreMinimal.h"

#include "BertaProcessTypes.generated.h"

UENUM(BlueprintType)
enum class EBertaProcessState : uint8
{
	Running,
	Completed,
	Canceled
};

UENUM(BlueprintType)
enum class EBertaProcessFinishReason : uint8
{
	Completed,
	Canceled
};

UENUM(BlueprintType)
enum class EBertaProcessLaunchError : uint8
{
	None,
	InvalidExecutable,
	InvalidWorkingDirectory,
	SubsystemUnavailable,
	LaunchFailed
};

USTRUCT(BlueprintType)
struct BERTAPROCESSBRIDGE_API FBertaProcessLaunchOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Process")
	FString ExecutablePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Process")
	FString Arguments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Process")
	FString WorkingDirectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Process")
	bool bHidden = true;
};

USTRUCT(BlueprintType)
struct BERTAPROCESSBRIDGE_API FBertaProcessResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	EBertaProcessFinishReason Reason = EBertaProcessFinishReason::Completed;

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	bool bHasExitCode = false;

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	int32 ExitCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	double DurationSeconds = 0.0;
};
