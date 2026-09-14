#include "Diagnostics/BertaComboGraphEffectContextCompatibility.h"

#include "Abilities/ComboGraphAbilitySystemGlobals.h"
#include "AbilitySystemGlobals.h"
#include "ComboGraphAbilityTypes.h"
#include "Templates/UniquePtr.h"

EBertaComboGraphEffectContextCompatibility UBertaComboGraphEffectContextCompatibilityLibrary::ClassifyGlobalsClass(const UClass* GlobalsClass)
{
	if (!GlobalsClass)
	{
		return EBertaComboGraphEffectContextCompatibility::Unknown;
	}
	return GlobalsClass->IsChildOf(UComboGraphAbilitySystemGlobals::StaticClass())
		? EBertaComboGraphEffectContextCompatibility::Compatible
		: EBertaComboGraphEffectContextCompatibility::Incompatible;
}

EBertaComboGraphEffectContextCompatibility UBertaComboGraphEffectContextCompatibilityLibrary::ClassifyContextStruct(const UScriptStruct* ContextStruct)
{
	if (!ContextStruct)
	{
		return EBertaComboGraphEffectContextCompatibility::Unknown;
	}
	return ContextStruct->IsChildOf(FComboGraphGameplayEffectContext::StaticStruct())
		? EBertaComboGraphEffectContextCompatibility::Compatible
		: EBertaComboGraphEffectContextCompatibility::Incompatible;
}

EBertaComboGraphEffectContextCompatibility UBertaComboGraphEffectContextCompatibilityLibrary::ClassifyRuntimeCompatibility(
	const EBertaComboGraphEffectContextCompatibility GlobalsClassCompatibility,
	const EBertaComboGraphEffectContextCompatibility AllocatedContextCompatibility)
{
	// Globals inheritance is useful configuration evidence, but Combo Graph runtime consumes
	// the context returned by AllocGameplayEffectContext(). That allocated struct is authoritative.
	(void)GlobalsClassCompatibility;
	return AllocatedContextCompatibility;
}

FBertaComboGraphEffectContextCompatibilityReport UBertaComboGraphEffectContextCompatibilityLibrary::InspectConfiguredGlobals()
{
	FBertaComboGraphEffectContextCompatibilityReport Report;
	const UAbilitySystemGlobals& Globals = UAbilitySystemGlobals::Get();
	const UClass* GlobalsClass = Globals.GetClass();
	Report.ConfiguredGlobalsClassPath = GetPathNameSafe(GlobalsClass);
	const EBertaComboGraphEffectContextCompatibility GlobalsClassCompatibility = ClassifyGlobalsClass(GlobalsClass);
	Report.bGlobalsClassCompatible = GlobalsClassCompatibility == EBertaComboGraphEffectContextCompatibility::Compatible;
	TUniquePtr<FGameplayEffectContext> AllocatedContext(Globals.AllocGameplayEffectContext());
	const UScriptStruct* ContextStruct = AllocatedContext ? AllocatedContext->GetScriptStruct() : nullptr;
	Report.AllocatedContextStructPath = GetPathNameSafe(ContextStruct);
	const EBertaComboGraphEffectContextCompatibility AllocatedContextCompatibility = ClassifyContextStruct(ContextStruct);
	Report.bAllocatedContextCompatible = AllocatedContextCompatibility == EBertaComboGraphEffectContextCompatibility::Compatible;
	Report.Compatibility = ClassifyRuntimeCompatibility(GlobalsClassCompatibility, AllocatedContextCompatibility);
	Report.bCueContainersSafe = Report.Compatibility == EBertaComboGraphEffectContextCompatibility::Compatible;
	if (AllocatedContextCompatibility == EBertaComboGraphEffectContextCompatibility::Compatible)
	{
		Report.Evidence = Report.bGlobalsClassCompatible
			? TEXT("Allocated context is Combo Graph compatible, and configured AbilitySystemGlobals derives from UComboGraphAbilitySystemGlobals.")
			: TEXT("Allocated context is Combo Graph compatible. Configured AbilitySystemGlobals does not derive from UComboGraphAbilitySystemGlobals, so Combo Graph Editor startup validation may still warn.");
	}
	else if (AllocatedContextCompatibility == EBertaComboGraphEffectContextCompatibility::Incompatible)
	{
		Report.Evidence = TEXT("Allocated context is not Combo Graph compatible; Cue Containers are runtime unsafe regardless of the configured AbilitySystemGlobals inheritance.");
	}
	else
	{
		Report.Evidence = TEXT("Allocated context compatibility could not be determined; Cue Container runtime safety is unknown.");
	}
	return Report;
}

FString UBertaComboGraphEffectContextCompatibilityLibrary::FormatCompatibilityReport(const FBertaComboGraphEffectContextCompatibilityReport& Report)
{
	const UEnum* Enum = StaticEnum<EBertaComboGraphEffectContextCompatibility>();
	return FString::Printf(TEXT("AbilitySystemGlobals=%s | AllocatedContext=%s | Compatibility=%s | CueContainersSafe=%s | %s"),
		*Report.ConfiguredGlobalsClassPath,
		*Report.AllocatedContextStructPath,
		*Enum->GetNameStringByValue(static_cast<int64>(Report.Compatibility)),
		Report.bCueContainersSafe ? TEXT("Yes") : TEXT("No"),
		*Report.Evidence);
}
