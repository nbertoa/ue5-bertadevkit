#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaGSCContractAudit.generated.h"

UENUM(BlueprintType)
enum class EBertaGSCContractFindingSeverity : uint8
{
	Error,
	Warning,
	Info
};

/** One objective or explicitly qualified finding from a GSC asset contract audit. */
USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXTEDITOR_API FBertaGSCContractFinding
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	EBertaGSCContractFindingSeverity Severity = EBertaGSCContractFindingSeverity::Info;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	FString AssetPath;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	FString RelatedAssetPath;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	FString PropertyPath;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	FString Evidence;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	FString StableKey;
};

USTRUCT(BlueprintType)
struct BERTAGASCOMPANIONEXTEDITOR_API FBertaGSCContractAuditReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	FString AuditedAssetPath;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	bool bSupportedAsset = false;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	int32 ErrorCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	int32 WarningCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	int32 InfoCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	TArray<FBertaGSCContractFinding> Findings;

	UPROPERTY(BlueprintReadOnly, Category = "Contract Audit")
	FString Summary;
};

/** Editor-only audit for one Ability Set, GSC Game Feature action, or Game Feature Data asset. */
UCLASS()
class BERTAGASCOMPANIONEXTEDITOR_API UBertaGSCContractAuditLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaGASCompanionExt|Validation", meta = (ReturnDisplayName = "Supported Asset"))
	static bool AuditAsset(UObject* Asset, FBertaGSCContractAuditReport& OutReport);

	UFUNCTION(BlueprintPure, Category = "BertaGASCompanionExt|Validation")
	static FString FormatContractAuditReport(const FBertaGSCContractAuditReport& Report);
};
