#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaComboGraphValidation.generated.h"

class UComboGraph;

UENUM(BlueprintType)
enum class EBertaComboGraphValidationSeverity : uint8
{
	Info,
	Warning,
	Error
};

USTRUCT(BlueprintType)
struct BERTACOMBOGRAPHEXTEDITOR_API FBertaComboGraphValidationFinding
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation") EBertaComboGraphValidationSeverity Severity = EBertaComboGraphValidationSeverity::Info;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") FString AssetPath;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") FString NodePath;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") FString EdgePath;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") FString PropertyPath;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") FString Code;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") FString Message;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") FString Evidence;
};

USTRUCT(BlueprintType)
struct BERTACOMBOGRAPHEXTEDITOR_API FBertaComboGraphValidationReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation") FString AssetPath;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") TArray<FBertaComboGraphValidationFinding> Findings;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") int32 ErrorCount = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") int32 WarningCount = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Validation") int32 InfoCount = 0;
};

UCLASS()
class BERTACOMBOGRAPHEXTEDITOR_API UBertaComboGraphValidationLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaComboGraphExt|Validation")
	static bool BuildValidationReport(UComboGraph* Graph, FBertaComboGraphValidationReport& OutReport);

	UFUNCTION(BlueprintPure, Category = "BertaComboGraphExt|Validation")
	static FString FormatValidationReport(const FBertaComboGraphValidationReport& Report);

	static void SortAndCount(FBertaComboGraphValidationReport& Report);
	static bool IsValidNormalizedRange(float Start, float End);
};
