#include "Validation/BertaComboGraphValidator.h"

#include "Graph/ComboGraph.h"
#include "Misc/DataValidation.h"
#include "Validation/BertaComboGraphValidation.h"

bool UBertaComboGraphValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UComboGraph>();
}

EDataValidationResult UBertaComboGraphValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
	FBertaComboGraphValidationReport Report;
	UBertaComboGraphValidationLibrary::BuildValidationReport(Cast<UComboGraph>(InAsset), Report);
	for (const FBertaComboGraphValidationFinding& Finding : Report.Findings)
	{
		const FText Text = FText::FromString(FString::Printf(TEXT("[%s] Node=%s Edge=%s Property=%s: %s Evidence: %s"),
			*Finding.Code, *Finding.NodePath, *Finding.EdgePath, *Finding.PropertyPath, *Finding.Message, *Finding.Evidence));
		if (Finding.Severity == EBertaComboGraphValidationSeverity::Error) AssetFails(InAsset, Text);
		else if (Finding.Severity == EBertaComboGraphValidationSeverity::Warning) AssetWarning(InAsset, Text);
	}
	if (Report.ErrorCount > 0) return EDataValidationResult::Invalid;
	AssetPasses(InAsset);
	return EDataValidationResult::Valid;
}
