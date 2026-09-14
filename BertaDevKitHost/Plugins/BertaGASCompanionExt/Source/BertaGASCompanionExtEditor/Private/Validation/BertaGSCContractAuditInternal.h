#pragma once

#include "Validation/BertaGSCContractAudit.h"

class UGSCAbilitySet;
class UGSCGameFeatureAction_AddAbilities;
class UGSCGameFeatureAction_AddInputMappingContext;
class UGameFeatureData;

namespace BertaGSCContractAudit
{
	struct FAbilityContractRecord
	{
		FString ClassPath;
		FString InputPath;
		int32 Level = 0;
		int32 Trigger = 0;
		FString PropertyPath;
	};

	bool IsExactDuplicate(const FAbilityContractRecord& Left, const FAbilityContractRecord& Right);
	bool HasConflictingSameAbilityConfiguration(const FAbilityContractRecord& Left, const FAbilityContractRecord& Right);
	bool SharesInputAcrossDifferentAbilities(const FAbilityContractRecord& Left, const FAbilityContractRecord& Right);

	void AddFinding(
		FBertaGSCContractAuditReport& Report,
		EBertaGSCContractFindingSeverity Severity,
		const FString& AssetPath,
		const FString& PropertyPath,
		const FString& Code,
		const FString& Message,
		const FString& Evidence = FString(),
		const FString& RelatedAssetPath = FString());
	void AuditAbilitySet(const UGSCAbilitySet& AbilitySet, FBertaGSCContractAuditReport& Report);
	void AuditAddAbilitiesAction(const UGSCGameFeatureAction_AddAbilities& Action, FBertaGSCContractAuditReport& Report);
	void AuditInputMappingAction(const UGSCGameFeatureAction_AddInputMappingContext& Action, FBertaGSCContractAuditReport& Report);
	void AuditGameFeatureData(const UGameFeatureData& Data, FBertaGSCContractAuditReport& Report);
	void Finalize(FBertaGSCContractAuditReport& Report);
	void SortFindings(TArray<FBertaGSCContractFinding>& Findings);
}
