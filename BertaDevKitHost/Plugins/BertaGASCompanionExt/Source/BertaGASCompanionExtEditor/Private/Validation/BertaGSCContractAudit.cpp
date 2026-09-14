#include "Validation/BertaGSCContractAudit.h"

#include "Validation/BertaGSCContractAuditInternal.h"
#include "Abilities/GSCAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "AttributeSet.h"
#include "Engine/DataTable.h"
#include "GameFeatureAction.h"
#include "GameFeatureData.h"
#include "GameFeatures/Actions/GSCGameFeatureAction_AddAbilities.h"
#include "GameFeatures/Actions/GSCGameFeatureAction_AddInputMappingContext.h"
#include "GameplayEffect.h"
#include "InputAction.h"
#include "InputMappingContext.h"

namespace
{
	bool IsClassInvalidForInstantiation(const UClass& Class)
	{
		return Class.HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists);
	}

	FString SeverityText(const EBertaGSCContractFindingSeverity Severity)
	{
		switch (Severity)
		{
		case EBertaGSCContractFindingSeverity::Error: return TEXT("Error");
		case EBertaGSCContractFindingSeverity::Warning: return TEXT("Warning");
		case EBertaGSCContractFindingSeverity::Info: return TEXT("Info");
		default: return TEXT("Info");
		}
	}

	void AuditAbilityMappings(
		const TArray<FGSCGameFeatureAbilityMapping>& Mappings,
		const FString& AssetPath,
		const FString& PropertyPrefix,
		FBertaGSCContractAuditReport& Report,
		TArray<BertaGSCContractAudit::FAbilityContractRecord>* OutRecords = nullptr)
	{
		TMap<FString, BertaGSCContractAudit::FAbilityContractRecord> FirstExact;
		TMap<FString, BertaGSCContractAudit::FAbilityContractRecord> FirstByClass;
		TMap<FString, BertaGSCContractAudit::FAbilityContractRecord> FirstByInput;
		for (int32 Index = 0; Index < Mappings.Num(); ++Index)
		{
			const FGSCGameFeatureAbilityMapping& Mapping = Mappings[Index];
			const FString PropertyPath = FString::Printf(TEXT("%s[%d]"), *PropertyPrefix, Index);
			const FString ClassPath = Mapping.AbilityType.ToSoftObjectPath().ToString();
			const FString InputPath = Mapping.InputAction.ToSoftObjectPath().ToString();
			UClass* AbilityClass = Mapping.AbilityType.LoadSynchronous();
			if (!AbilityClass)
			{
				BertaGSCContractAudit::AddFinding(
					Report, EBertaGSCContractFindingSeverity::Error, AssetPath, PropertyPath + TEXT(".AbilityType"),
					TEXT("UnresolvedAbility"), TEXT("AbilityType is null or cannot be resolved."), ClassPath);
			}
			else if (IsClassInvalidForInstantiation(*AbilityClass))
			{
				BertaGSCContractAudit::AddFinding(
					Report, EBertaGSCContractFindingSeverity::Error, AssetPath, PropertyPath + TEXT(".AbilityType"),
					TEXT("NonInstantiableAbility"), TEXT("Ability class cannot be instantiated."), AbilityClass->GetPathName());
			}
			else if (AbilityClass->HasAnyClassFlags(CLASS_Deprecated))
			{
				BertaGSCContractAudit::AddFinding(
					Report, EBertaGSCContractFindingSeverity::Warning, AssetPath, PropertyPath + TEXT(".AbilityType"),
					TEXT("DeprecatedAbility"), TEXT("Ability class is deprecated."), AbilityClass->GetPathName());
			}

			if (Mapping.Level <= 0)
			{
				BertaGSCContractAudit::AddFinding(
					Report, EBertaGSCContractFindingSeverity::Error, AssetPath, PropertyPath + TEXT(".Level"),
					TEXT("InvalidAbilityLevel"), TEXT("Ability level must be positive."), FString::FromInt(Mapping.Level));
			}
			if (!Mapping.InputAction.IsNull() && !Mapping.InputAction.LoadSynchronous())
			{
				BertaGSCContractAudit::AddFinding(
					Report, EBertaGSCContractFindingSeverity::Error, AssetPath, PropertyPath + TEXT(".InputAction"),
					TEXT("UnresolvedInputAction"), TEXT("Configured InputAction cannot be resolved."), InputPath);
			}

			const BertaGSCContractAudit::FAbilityContractRecord Record{
				ClassPath,
				InputPath,
				Mapping.Level,
				InputPath.IsEmpty() ? 0 : static_cast<int32>(Mapping.TriggerEvent),
				PropertyPath};
			if (OutRecords)
			{
				OutRecords->Add(Record);
			}
			const FString ExactKey = FString::Printf(
				TEXT("%s|%d|%s|%d"), *ClassPath, Mapping.Level, *InputPath, Record.Trigger);
			if (const BertaGSCContractAudit::FAbilityContractRecord* First = FirstExact.Find(ExactKey);
				First && BertaGSCContractAudit::IsExactDuplicate(*First, Record))
			{
				BertaGSCContractAudit::AddFinding(
					Report, EBertaGSCContractFindingSeverity::Error, AssetPath, PropertyPath,
					TEXT("DuplicateAbilityMapping"), TEXT("Exact duplicate ability grant mapping."), ExactKey);
			}
			FirstExact.FindOrAdd(ExactKey, Record);

			if (!ClassPath.IsEmpty())
			{
				if (const BertaGSCContractAudit::FAbilityContractRecord* First = FirstByClass.Find(ClassPath);
					First && BertaGSCContractAudit::HasConflictingSameAbilityConfiguration(*First, Record))
				{
					BertaGSCContractAudit::AddFinding(
						Report, EBertaGSCContractFindingSeverity::Warning, AssetPath, PropertyPath,
						TEXT("ConflictingAbilityMapping"),
						TEXT("The same exact ability has conflicting level or input configuration in this contract."),
						FString::Printf(TEXT("First=%s; Current=%s"), *First->PropertyPath, *PropertyPath));
				}
				else if (!First)
				{
					FirstByClass.Add(ClassPath, Record);
				}
			}

			if (!InputPath.IsEmpty())
			{
				if (const BertaGSCContractAudit::FAbilityContractRecord* First = FirstByInput.Find(InputPath);
					First && BertaGSCContractAudit::SharesInputAcrossDifferentAbilities(*First, Record))
				{
					BertaGSCContractAudit::AddFinding(
						Report, EBertaGSCContractFindingSeverity::Warning, AssetPath, PropertyPath + TEXT(".InputAction"),
						TEXT("SharedInputAction"),
						TEXT("Multiple abilities share one InputAction. GSC supports a binding stack; verify ordering is intentional."),
						FString::Printf(TEXT("First=%s; Input=%s"), *First->PropertyPath, *InputPath));
				}
				else if (!First)
				{
					FirstByInput.Add(InputPath, Record);
				}
			}
		}
	}

	void AuditAttributeMappings(
		const TArray<FGSCGameFeatureAttributeSetMapping>& Mappings,
		const FString& AssetPath,
		const FString& PropertyPrefix,
		FBertaGSCContractAuditReport& Report)
	{
		TSet<FString> ExactKeys;
		for (int32 Index = 0; Index < Mappings.Num(); ++Index)
		{
			const FGSCGameFeatureAttributeSetMapping& Mapping = Mappings[Index];
			const FString PropertyPath = FString::Printf(TEXT("%s[%d]"), *PropertyPrefix, Index);
			const FString ClassPath = Mapping.AttributeSet.ToSoftObjectPath().ToString();
			const FString TablePath = Mapping.InitializationData.ToSoftObjectPath().ToString();
			UClass* AttributeClass = Mapping.AttributeSet.LoadSynchronous();
			if (!AttributeClass)
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath + TEXT(".AttributeSet"), TEXT("UnresolvedAttributeSet"),
					TEXT("AttributeSet is null or cannot be resolved."), ClassPath);
			}
			else if (IsClassInvalidForInstantiation(*AttributeClass))
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath + TEXT(".AttributeSet"), TEXT("NonInstantiableAttributeSet"),
					TEXT("AttributeSet class cannot be instantiated."), AttributeClass->GetPathName());
			}

			if (!Mapping.InitializationData.IsNull())
			{
				UDataTable* Table = Mapping.InitializationData.LoadSynchronous();
				if (!Table)
				{
					BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
						PropertyPath + TEXT(".InitializationData"), TEXT("UnresolvedInitializationData"),
						TEXT("Configured InitializationData cannot be resolved."), TablePath);
				}
				else if (Table->GetRowStruct() != FAttributeMetaData::StaticStruct())
				{
					BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
						PropertyPath + TEXT(".InitializationData"), TEXT("InvalidInitializationDataRowStruct"),
						TEXT("InitializationData must use the public GSC AttributeMetaData row contract."),
						GetPathNameSafe(Table->GetRowStruct()), TablePath);
				}
			}

			const FString ExactKey = ClassPath + TEXT("|") + TablePath;
			if (ExactKeys.Contains(ExactKey))
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath, TEXT("DuplicateAttributeMapping"), TEXT("Exact duplicate AttributeSet grant mapping."), ExactKey);
			}
			ExactKeys.Add(ExactKey);
		}
	}

	void AuditEffectMappings(
		const TArray<FGSCGameFeatureGameplayEffectMapping>& Mappings,
		const FString& AssetPath,
		const FString& PropertyPrefix,
		FBertaGSCContractAuditReport& Report)
	{
		TSet<FString> ExactKeys;
		for (int32 Index = 0; Index < Mappings.Num(); ++Index)
		{
			const FGSCGameFeatureGameplayEffectMapping& Mapping = Mappings[Index];
			const FString PropertyPath = FString::Printf(TEXT("%s[%d]"), *PropertyPrefix, Index);
			const FString ClassPath = Mapping.EffectType.ToSoftObjectPath().ToString();
			UClass* EffectClass = Mapping.EffectType.LoadSynchronous();
			if (!EffectClass)
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath + TEXT(".EffectType"), TEXT("UnresolvedGameplayEffect"),
					TEXT("GameplayEffect is null or cannot be resolved."), ClassPath);
			}
			else if (IsClassInvalidForInstantiation(*EffectClass))
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath + TEXT(".EffectType"), TEXT("NonInstantiableGameplayEffect"),
					TEXT("GameplayEffect class cannot be instantiated."), EffectClass->GetPathName());
			}
			if (!FMath::IsFinite(Mapping.Level))
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath + TEXT(".Level"), TEXT("InvalidGameplayEffectLevel"),
					TEXT("GameplayEffect level must be finite."));
			}
			const FString ExactKey = ClassPath + TEXT("|") + FString::SanitizeFloat(Mapping.Level);
			if (ExactKeys.Contains(ExactKey))
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath, TEXT("DuplicateGameplayEffectMapping"), TEXT("Exact duplicate GameplayEffect grant mapping."), ExactKey);
			}
			ExactKeys.Add(ExactKey);
		}
	}

	void AuditAddAbilitiesEntry(
		const FGSCGameFeatureAbilitiesEntry& Entry,
		const FString& AssetPath,
		const FString& Prefix,
		FBertaGSCContractAuditReport& Report,
		TArray<BertaGSCContractAudit::FAbilityContractRecord>* OutEffectiveInputRecords = nullptr)
	{
		const FString ActorPath = Entry.ActorClass.ToSoftObjectPath().ToString();
		UClass* ActorClass = Entry.ActorClass.LoadSynchronous();
		if (!ActorClass)
		{
			BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
				Prefix + TEXT(".ActorClass"), TEXT("UnresolvedActorClass"),
				TEXT("Target ActorClass is null or cannot be resolved."), ActorPath);
		}

		TArray<BertaGSCContractAudit::FAbilityContractRecord> DirectRecords;
		AuditAbilityMappings(Entry.GrantedAbilities, AssetPath, Prefix + TEXT(".GrantedAbilities"), Report, &DirectRecords);
		AuditAttributeMappings(Entry.GrantedAttributes, AssetPath, Prefix + TEXT(".GrantedAttributes"), Report);
		AuditEffectMappings(Entry.GrantedEffects, AssetPath, Prefix + TEXT(".GrantedEffects"), Report);
		if (OutEffectiveInputRecords)
		{
			OutEffectiveInputRecords->Append(DirectRecords);
		}

		TSet<FString> AbilitySetPaths;
		TMap<FString, BertaGSCContractAudit::FAbilityContractRecord> DirectAbilities;
		for (const BertaGSCContractAudit::FAbilityContractRecord& Record : DirectRecords)
		{
			DirectAbilities.FindOrAdd(Record.ClassPath, Record);
		}
		for (int32 SetIndex = 0; SetIndex < Entry.GrantedAbilitySets.Num(); ++SetIndex)
		{
			const TSoftObjectPtr<UGSCAbilitySet>& SoftSet = Entry.GrantedAbilitySets[SetIndex];
			const FString SetPath = SoftSet.ToSoftObjectPath().ToString();
			const FString PropertyPath = FString::Printf(TEXT("%s.GrantedAbilitySets[%d]"), *Prefix, SetIndex);
			const UGSCAbilitySet* AbilitySet = SoftSet.LoadSynchronous();
			if (!AbilitySet)
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath, TEXT("UnresolvedAbilitySet"), TEXT("Ability Set is null or cannot be resolved."), SetPath);
				continue;
			}
			if (AbilitySetPaths.Contains(SetPath))
			{
				BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath,
					PropertyPath, TEXT("DuplicateAbilitySet"),
					TEXT("The same Ability Set is applied more than once to this Game Feature entry."), SetPath, SetPath);
			}
			AbilitySetPaths.Add(SetPath);
			BertaGSCContractAudit::AuditAbilitySet(*AbilitySet, Report);

			for (int32 AbilityIndex = 0; AbilityIndex < AbilitySet->GrantedAbilities.Num(); ++AbilityIndex)
			{
				const FGSCGameFeatureAbilityMapping& Mapping = AbilitySet->GrantedAbilities[AbilityIndex];
				const FString AbilityPath = Mapping.AbilityType.ToSoftObjectPath().ToString();
				if (const BertaGSCContractAudit::FAbilityContractRecord* Direct = DirectAbilities.Find(AbilityPath))
				{
					const FString SetInputPath = Mapping.InputAction.ToSoftObjectPath().ToString();
					const int32 SetTrigger = SetInputPath.IsEmpty()
						? 0
						: static_cast<int32>(Mapping.TriggerEvent);
					BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Warning, AssetPath,
						PropertyPath, TEXT("DirectAbilitySetOverlap"),
						TEXT("This entry grants the same exact ability directly and through an Ability Set."),
						FString::Printf(TEXT("Ability=%s; Direct=%s; AbilitySetEntry=GrantedAbilities[%d]"), *AbilityPath, *Direct->PropertyPath, AbilityIndex), SetPath);
					if (Direct->Level != Mapping.Level
						|| Direct->InputPath != SetInputPath
						|| Direct->Trigger != SetTrigger)
					{
						BertaGSCContractAudit::AddFinding(Report, EBertaGSCContractFindingSeverity::Warning, AssetPath,
							PropertyPath, TEXT("ConflictingDirectAbilitySetGrant"),
							TEXT("Direct and Ability Set grants for the same ability use conflicting level or input configuration."),
							FString::Printf(
								TEXT("DirectLevel=%d; SetLevel=%d; DirectInput=%s; SetInput=%s"),
								Direct->Level, Mapping.Level, *Direct->InputPath, *SetInputPath),
							SetPath);
					}
				}
				if (OutEffectiveInputRecords)
				{
					const FString InputPath = Mapping.InputAction.ToSoftObjectPath().ToString();
					OutEffectiveInputRecords->Add(BertaGSCContractAudit::FAbilityContractRecord{
						AbilityPath,
						InputPath,
						Mapping.Level,
						InputPath.IsEmpty() ? 0 : static_cast<int32>(Mapping.TriggerEvent),
						PropertyPath + FString::Printf(TEXT("->GrantedAbilities[%d]"), AbilityIndex)});
				}
			}
		}
	}
}

namespace BertaGSCContractAudit
{
	bool IsExactDuplicate(const FAbilityContractRecord& Left, const FAbilityContractRecord& Right)
	{
		return Left.ClassPath == Right.ClassPath
			&& Left.Level == Right.Level
			&& Left.InputPath == Right.InputPath
			&& Left.Trigger == Right.Trigger;
	}

	bool HasConflictingSameAbilityConfiguration(
		const FAbilityContractRecord& Left,
		const FAbilityContractRecord& Right)
	{
		return !Left.ClassPath.IsEmpty()
			&& Left.ClassPath == Right.ClassPath
			&& !IsExactDuplicate(Left, Right);
	}

	bool SharesInputAcrossDifferentAbilities(
		const FAbilityContractRecord& Left,
		const FAbilityContractRecord& Right)
	{
		return !Left.InputPath.IsEmpty()
			&& Left.InputPath == Right.InputPath
			&& Left.ClassPath != Right.ClassPath;
	}

	void AddFinding(
		FBertaGSCContractAuditReport& Report,
		const EBertaGSCContractFindingSeverity Severity,
		const FString& AssetPath,
		const FString& PropertyPath,
		const FString& Code,
		const FString& Message,
		const FString& Evidence,
		const FString& RelatedAssetPath)
	{
		FBertaGSCContractFinding& Finding = Report.Findings.AddDefaulted_GetRef();
		Finding.Severity = Severity;
		Finding.AssetPath = AssetPath;
		Finding.RelatedAssetPath = RelatedAssetPath;
		Finding.PropertyPath = PropertyPath;
		Finding.Message = Message;
		Finding.Evidence = Evidence;
		Finding.StableKey = FString::Printf(
			TEXT("%d|%s|%s|%s|%s"), static_cast<int32>(Severity), *AssetPath, *PropertyPath, *Code, *RelatedAssetPath);
	}

	void AuditAbilitySet(const UGSCAbilitySet& AbilitySet, FBertaGSCContractAuditReport& Report)
	{
		const FString AssetPath = AbilitySet.GetPathName();
		AuditAbilityMappings(AbilitySet.GrantedAbilities, AssetPath, TEXT("GrantedAbilities"), Report);
		AuditAttributeMappings(AbilitySet.GrantedAttributes, AssetPath, TEXT("GrantedAttributes"), Report);
		AuditEffectMappings(AbilitySet.GrantedEffects, AssetPath, TEXT("GrantedEffects"), Report);
		for (const FGameplayTag& Tag : AbilitySet.OwnedTags.GetGameplayTagArray())
		{
			if (!Tag.IsValid())
			{
				AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath, TEXT("OwnedTags"),
					TEXT("InvalidOwnedTag"), TEXT("OwnedTags contains an invalid Gameplay Tag reference."));
			}
		}
	}

	void AuditAddAbilitiesAction(
		const UGSCGameFeatureAction_AddAbilities& Action,
		FBertaGSCContractAuditReport& Report)
	{
		const FString AssetPath = Action.GetPathName();
		for (int32 Index = 0; Index < Action.AbilitiesList.Num(); ++Index)
		{
			AuditAddAbilitiesEntry(
				Action.AbilitiesList[Index],
				AssetPath,
				FString::Printf(TEXT("AbilitiesList[%d]"), Index),
				Report);
		}
	}

	void AuditInputMappingAction(
		const UGSCGameFeatureAction_AddInputMappingContext& Action,
		FBertaGSCContractAuditReport& Report)
	{
		const FString MappingPath = Action.InputMapping.ToSoftObjectPath().ToString();
		if (!Action.InputMapping.LoadSynchronous())
		{
			AddFinding(Report, EBertaGSCContractFindingSeverity::Error, Action.GetPathName(), TEXT("InputMapping"),
				TEXT("UnresolvedInputMappingContext"), TEXT("InputMapping is null or cannot be resolved."), MappingPath);
		}
	}

	void AuditGameFeatureData(const UGameFeatureData& Data, FBertaGSCContractAuditReport& Report)
	{
		const FString AssetPath = Data.GetPathName();
		TSet<FString> MappingContexts;
		TArray<const UInputMappingContext*> LoadedContexts;
		TArray<FAbilityContractRecord> EffectiveInputRecords;
		for (int32 ActionIndex = 0; ActionIndex < Data.GetActions().Num(); ++ActionIndex)
		{
			const UGameFeatureAction* Action = Data.GetActions()[ActionIndex];
			if (const UGSCGameFeatureAction_AddAbilities* AddAbilities = Cast<UGSCGameFeatureAction_AddAbilities>(Action))
			{
				for (int32 EntryIndex = 0; EntryIndex < AddAbilities->AbilitiesList.Num(); ++EntryIndex)
				{
					AuditAddAbilitiesEntry(
						AddAbilities->AbilitiesList[EntryIndex],
						AssetPath,
						FString::Printf(TEXT("Actions[%d].AbilitiesList[%d]"), ActionIndex, EntryIndex),
						Report,
						&EffectiveInputRecords);
				}
			}
			else if (const UGSCGameFeatureAction_AddInputMappingContext* AddInput =
				Cast<UGSCGameFeatureAction_AddInputMappingContext>(Action))
			{
				const FString MappingPath = AddInput->InputMapping.ToSoftObjectPath().ToString();
				const FString PropertyPath = FString::Printf(TEXT("Actions[%d].InputMapping"), ActionIndex);
				const UInputMappingContext* Context = AddInput->InputMapping.LoadSynchronous();
				if (!Context)
				{
					AddFinding(Report, EBertaGSCContractFindingSeverity::Error, AssetPath, PropertyPath,
						TEXT("UnresolvedInputMappingContext"), TEXT("InputMapping is null or cannot be resolved."), MappingPath);
				}
				else
				{
					if (MappingContexts.Contains(MappingPath))
					{
						const bool bRegistrationCounted = Context->GetRegistrationTrackingMode()
							== EMappingContextRegistrationTrackingMode::CountRegistrations;
						AddFinding(Report,
							bRegistrationCounted ? EBertaGSCContractFindingSeverity::Info : EBertaGSCContractFindingSeverity::Warning,
							AssetPath, PropertyPath,
							TEXT("DuplicateInputMappingContext"),
							bRegistrationCounted
								? TEXT("The same tracked Input Mapping Context is added multiple times by this Game Feature.")
								: TEXT("The same untracked Input Mapping Context is added multiple times; first removal can remove it."),
							bRegistrationCounted ? TEXT("RegistrationTrackingMode=CountRegistrations") : TEXT("RegistrationTrackingMode=Untracked"),
							MappingPath);
					}
					MappingContexts.Add(MappingPath);
					LoadedContexts.Add(Context);
				}
			}
		}

		for (const FAbilityContractRecord& Record : EffectiveInputRecords)
		{
			if (Record.InputPath.IsEmpty())
			{
				continue;
			}
			const UInputAction* InputAction = LoadObject<UInputAction>(nullptr, *Record.InputPath);
			if (!InputAction)
			{
				continue;
			}
			const bool bFound = LoadedContexts.ContainsByPredicate([InputAction](const UInputMappingContext* Context)
			{
				return Context && Context->HasMappingForInputAction(InputAction);
			});
			if (!bFound)
			{
				AddFinding(Report, EBertaGSCContractFindingSeverity::Warning, AssetPath, Record.PropertyPath + TEXT(".InputAction"),
					TEXT("InputActionNotMappedInSameGameFeature"),
					TEXT("No GSC Input Mapping action in this Game Feature maps the configured InputAction."),
					TEXT("Another project system may legitimately provide the mapping; this is not proof of a broken input contract."),
					Record.InputPath);
			}
		}
	}

	void SortFindings(TArray<FBertaGSCContractFinding>& Findings)
	{
		Findings.Sort([](const FBertaGSCContractFinding& Left, const FBertaGSCContractFinding& Right)
		{
			return Left.StableKey.Compare(Right.StableKey, ESearchCase::CaseSensitive) < 0;
		});
	}

	void Finalize(FBertaGSCContractAuditReport& Report)
	{
		SortFindings(Report.Findings);
		for (int32 Index = Report.Findings.Num() - 1; Index > 0; --Index)
		{
			if (Report.Findings[Index].StableKey == Report.Findings[Index - 1].StableKey
				&& Report.Findings[Index].Message == Report.Findings[Index - 1].Message
				&& Report.Findings[Index].Evidence == Report.Findings[Index - 1].Evidence)
			{
				Report.Findings.RemoveAt(Index);
			}
		}
		Report.ErrorCount = 0;
		Report.WarningCount = 0;
		Report.InfoCount = 0;
		for (const FBertaGSCContractFinding& Finding : Report.Findings)
		{
			switch (Finding.Severity)
			{
			case EBertaGSCContractFindingSeverity::Error: ++Report.ErrorCount; break;
			case EBertaGSCContractFindingSeverity::Warning: ++Report.WarningCount; break;
			case EBertaGSCContractFindingSeverity::Info: ++Report.InfoCount; break;
			default: break;
			}
		}
		Report.Summary = UBertaGSCContractAuditLibrary::FormatContractAuditReport(Report);
	}
}

bool UBertaGSCContractAuditLibrary::AuditAsset(UObject* Asset, FBertaGSCContractAuditReport& OutReport)
{
	OutReport = FBertaGSCContractAuditReport();
	OutReport.AuditedAssetPath = GetPathNameSafe(Asset);
	if (const UGSCAbilitySet* AbilitySet = Cast<UGSCAbilitySet>(Asset))
	{
		OutReport.bSupportedAsset = true;
		BertaGSCContractAudit::AuditAbilitySet(*AbilitySet, OutReport);
	}
	else if (const UGSCGameFeatureAction_AddAbilities* AddAbilities = Cast<UGSCGameFeatureAction_AddAbilities>(Asset))
	{
		OutReport.bSupportedAsset = true;
		BertaGSCContractAudit::AuditAddAbilitiesAction(*AddAbilities, OutReport);
	}
	else if (const UGSCGameFeatureAction_AddInputMappingContext* AddInput = Cast<UGSCGameFeatureAction_AddInputMappingContext>(Asset))
	{
		OutReport.bSupportedAsset = true;
		BertaGSCContractAudit::AuditInputMappingAction(*AddInput, OutReport);
	}
	else if (const UGameFeatureData* Data = Cast<UGameFeatureData>(Asset))
	{
		OutReport.bSupportedAsset = true;
		BertaGSCContractAudit::AuditGameFeatureData(*Data, OutReport);
	}
	BertaGSCContractAudit::Finalize(OutReport);
	return OutReport.bSupportedAsset;
}

FString UBertaGSCContractAuditLibrary::FormatContractAuditReport(const FBertaGSCContractAuditReport& Report)
{
	TArray<FString> Lines;
	Lines.Add(FString::Printf(
		TEXT("Asset: %s | Supported: %s | Errors: %d | Warnings: %d | Info: %d"),
		*Report.AuditedAssetPath,
		Report.bSupportedAsset ? TEXT("Yes") : TEXT("No"),
		Report.ErrorCount,
		Report.WarningCount,
		Report.InfoCount));
	for (const FBertaGSCContractFinding& Finding : Report.Findings)
	{
		Lines.Add(FString::Printf(
			TEXT("[%s] %s | %s | %s%s%s"),
			*SeverityText(Finding.Severity),
			*Finding.StableKey,
			*Finding.Message,
			*Finding.Evidence,
			Finding.RelatedAssetPath.IsEmpty() ? TEXT("") : TEXT(" | Related="),
			*Finding.RelatedAssetPath));
	}
	return FString::Join(Lines, TEXT("\n"));
}
