#include "Validation/BertaGSCAbilitySetValidator.h"

#include "Abilities/GSCAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "AttributeSet.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "InputAction.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "BertaGSCAbilitySetValidator"

namespace BertaGSCAbilitySetValidation
{
	bool IsGameplayEffectLevelValid(const float Level)
	{
		return FMath::IsFinite(Level);
	}

	bool IsInitializationDataResolvable(const TSoftObjectPtr<UDataTable>& InitializationData)
	{
		return InitializationData.IsNull() || InitializationData.LoadSynchronous() != nullptr;
	}
}

namespace
{
	bool IsClassInvalidForInstantiation(const UClass& Class)
	{
		return Class.HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists);
	}

	FString MappingKey(const FString& ClassPath, const FString& SecondaryPath, const FString& Value)
	{
		return FString::Printf(TEXT("%s|%s|%s"), *ClassPath, *SecondaryPath, *Value);
	}
}

bool UBertaGSCAbilitySetValidator::CanValidateAsset_Implementation(
	const FAssetData& InAssetData,
	UObject* InAsset,
	FDataValidationContext& InContext) const
{
	return InAsset && InAsset->IsA<UGSCAbilitySet>();
}

EDataValidationResult UBertaGSCAbilitySetValidator::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData,
	UObject* InAsset,
	FDataValidationContext& InContext)
{
	const UGSCAbilitySet* AbilitySet = CastChecked<UGSCAbilitySet>(InAsset);
	const FString AssetPath = InAssetData.GetObjectPathString();
	bool bHasErrors = false;
	TSet<FString> ExactAbilityMappings;
	TMap<FString, FString> AbilityByInputAction;

	for (int32 Index = 0; Index < AbilitySet->GrantedAbilities.Num(); ++Index)
	{
		const FGSCGameFeatureAbilityMapping& Mapping = AbilitySet->GrantedAbilities[Index];
		UClass* AbilityClass = Mapping.AbilityType.LoadSynchronous();
		if (!AbilityClass)
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("MissingAbility", "{0}: GrantedAbilities[{1}] has a null or unresolved AbilityType."),
				FText::FromString(AssetPath), FText::AsNumber(Index)));
			bHasErrors = true;
			continue;
		}
		if (Mapping.Level <= 0)
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("InvalidAbilityLevel", "{0}: GrantedAbilities[{1}] ({2}) has non-positive level {3}."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(AbilityClass->GetPathName()), FText::AsNumber(Mapping.Level)));
			bHasErrors = true;
		}
		if (IsClassInvalidForInstantiation(*AbilityClass))
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("InvalidAbilityClass", "{0}: GrantedAbilities[{1}] references non-instantiable class {2}."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(AbilityClass->GetPathName())));
			bHasErrors = true;
		}
		else if (AbilityClass->HasAnyClassFlags(CLASS_Deprecated))
		{
			AssetWarning(InAsset, FText::Format(
				LOCTEXT("DeprecatedAbilityClass", "{0}: GrantedAbilities[{1}] references deprecated class {2}."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(AbilityClass->GetPathName())));
		}

		const FString InputPath = Mapping.InputAction.ToSoftObjectPath().ToString();
		if (!Mapping.InputAction.IsNull() && !Mapping.InputAction.LoadSynchronous())
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("InvalidInputAction", "{0}: GrantedAbilities[{1}] ({2}) claims InputAction {3}, but it cannot be resolved."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(AbilityClass->GetPathName()), FText::FromString(InputPath)));
			bHasErrors = true;
		}

		const FString ExactKey = MappingKey(
			AbilityClass->GetPathName(),
			InputPath,
			InputPath.IsEmpty()
				? FString::FromInt(Mapping.Level)
				: FString::Printf(TEXT("%d|%d"), Mapping.Level, static_cast<int32>(Mapping.TriggerEvent)));
		if (ExactAbilityMappings.Contains(ExactKey))
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("DuplicateAbility", "{0}: GrantedAbilities[{1}] is an exact duplicate mapping for {2}."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(AbilityClass->GetPathName())));
			bHasErrors = true;
		}
		ExactAbilityMappings.Add(ExactKey);

		if (!InputPath.IsEmpty())
		{
			if (const FString* ExistingAbility = AbilityByInputAction.Find(InputPath);
				ExistingAbility && *ExistingAbility != AbilityClass->GetPathName())
			{
				AssetWarning(InAsset, FText::Format(
					LOCTEXT("SharedInputAction", "{0}: GrantedAbilities[{1}] shares InputAction {2} with {3}. GAS Companion supports stacks for one action; verify the ordering is intentional."),
					FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(InputPath), FText::FromString(*ExistingAbility)));
			}
			else
			{
				AbilityByInputAction.Add(InputPath, AbilityClass->GetPathName());
			}
		}
	}

	TSet<FString> ExactAttributeMappings;
	for (int32 Index = 0; Index < AbilitySet->GrantedAttributes.Num(); ++Index)
	{
		const FGSCGameFeatureAttributeSetMapping& Mapping = AbilitySet->GrantedAttributes[Index];
		const FString InitializationDataPath = Mapping.InitializationData.ToSoftObjectPath().ToString();
		if (!BertaGSCAbilitySetValidation::IsInitializationDataResolvable(Mapping.InitializationData))
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("InvalidInitializationData", "{0}: GrantedAttributes[{1}] references InitializationData {2}, but it cannot be resolved."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(InitializationDataPath)));
			bHasErrors = true;
		}

		UClass* AttributeClass = Mapping.AttributeSet.LoadSynchronous();
		if (!AttributeClass)
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("MissingAttribute", "{0}: GrantedAttributes[{1}] has a null or unresolved AttributeSet."),
				FText::FromString(AssetPath), FText::AsNumber(Index)));
			bHasErrors = true;
			continue;
		}
		if (IsClassInvalidForInstantiation(*AttributeClass))
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("InvalidAttributeClass", "{0}: GrantedAttributes[{1}] references non-instantiable class {2}."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(AttributeClass->GetPathName())));
			bHasErrors = true;
		}

		const FString ExactKey = MappingKey(
			AttributeClass->GetPathName(), InitializationDataPath, TEXT(""));
		if (ExactAttributeMappings.Contains(ExactKey))
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("DuplicateAttribute", "{0}: GrantedAttributes[{1}] is an exact duplicate mapping for {2}."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(AttributeClass->GetPathName())));
			bHasErrors = true;
		}
		ExactAttributeMappings.Add(ExactKey);
	}

	TSet<FString> ExactEffectMappings;
	for (int32 Index = 0; Index < AbilitySet->GrantedEffects.Num(); ++Index)
	{
		const FGSCGameFeatureGameplayEffectMapping& Mapping = AbilitySet->GrantedEffects[Index];
		UClass* EffectClass = Mapping.EffectType.LoadSynchronous();
		if (!EffectClass)
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("MissingEffect", "{0}: GrantedEffects[{1}] has a null or unresolved EffectType."),
				FText::FromString(AssetPath), FText::AsNumber(Index)));
			bHasErrors = true;
			continue;
		}
		if (!BertaGSCAbilitySetValidation::IsGameplayEffectLevelValid(Mapping.Level))
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("InvalidEffectLevel", "{0}: GrantedEffects[{1}] ({2}) has a non-finite level."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(EffectClass->GetPathName())));
			bHasErrors = true;
		}
		if (IsClassInvalidForInstantiation(*EffectClass))
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("InvalidEffectClass", "{0}: GrantedEffects[{1}] references non-instantiable class {2}."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(EffectClass->GetPathName())));
			bHasErrors = true;
		}

		const FString ExactKey = MappingKey(
			EffectClass->GetPathName(), TEXT(""), FString::SanitizeFloat(Mapping.Level));
		if (ExactEffectMappings.Contains(ExactKey))
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("DuplicateEffect", "{0}: GrantedEffects[{1}] is an exact duplicate mapping for {2} at level {3}."),
				FText::FromString(AssetPath), FText::AsNumber(Index), FText::FromString(EffectClass->GetPathName()), FText::AsNumber(Mapping.Level)));
			bHasErrors = true;
		}
		ExactEffectMappings.Add(ExactKey);
	}

	for (const FGameplayTag& Tag : AbilitySet->OwnedTags.GetGameplayTagArray())
	{
		if (!Tag.IsValid())
		{
			AssetFails(InAsset, FText::Format(
				LOCTEXT("InvalidOwnedTag", "{0}: OwnedTags contains an invalid Gameplay Tag reference."),
				FText::FromString(AssetPath)));
			bHasErrors = true;
		}
	}

	if (bHasErrors)
	{
		return EDataValidationResult::Invalid;
	}

	AssetPasses(InAsset);
	return EDataValidationResult::Valid;
}

#undef LOCTEXT_NAMESPACE
