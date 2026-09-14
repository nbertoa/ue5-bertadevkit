#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaComboGraphEffectContextCompatibility.generated.h"

UENUM(BlueprintType)
enum class EBertaComboGraphEffectContextCompatibility : uint8
{
	Compatible,
	Incompatible,
	Unknown
};

USTRUCT(BlueprintType)
struct BERTACOMBOGRAPHEXT_API FBertaComboGraphEffectContextCompatibilityReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	FString ConfiguredGlobalsClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	FString AllocatedContextStructPath;

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	bool bGlobalsClassCompatible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	bool bAllocatedContextCompatible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	EBertaComboGraphEffectContextCompatibility Compatibility = EBertaComboGraphEffectContextCompatibility::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	bool bCueContainersSafe = false;

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	FString Evidence;
};

/** Read-only inspection of the AbilitySystemGlobals inheritance contract required by Combo Graph cue containers. */
UCLASS()
class BERTACOMBOGRAPHEXT_API UBertaComboGraphEffectContextCompatibilityLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "BertaComboGraphExt|Diagnostics")
	static FBertaComboGraphEffectContextCompatibilityReport InspectConfiguredGlobals();

	UFUNCTION(BlueprintPure, Category = "BertaComboGraphExt|Diagnostics")
	static FString FormatCompatibilityReport(const FBertaComboGraphEffectContextCompatibilityReport& Report);

	static EBertaComboGraphEffectContextCompatibility ClassifyGlobalsClass(const UClass* GlobalsClass);
	static EBertaComboGraphEffectContextCompatibility ClassifyContextStruct(const UScriptStruct* ContextStruct);
};
