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

FBertaComboGraphEffectContextCompatibilityReport UBertaComboGraphEffectContextCompatibilityLibrary::InspectConfiguredGlobals()
{
	FBertaComboGraphEffectContextCompatibilityReport Report;
	const UAbilitySystemGlobals& Globals = UAbilitySystemGlobals::Get();
	const UClass* GlobalsClass = Globals.GetClass();
	Report.ConfiguredGlobalsClassPath = GetPathNameSafe(GlobalsClass);
	Report.bGlobalsClassCompatible = ClassifyGlobalsClass(GlobalsClass) == EBertaComboGraphEffectContextCompatibility::Compatible;
	TUniquePtr<FGameplayEffectContext> AllocatedContext(Globals.AllocGameplayEffectContext());
	const UScriptStruct* ContextStruct = AllocatedContext ? AllocatedContext->GetScriptStruct() : nullptr;
	Report.AllocatedContextStructPath = GetPathNameSafe(ContextStruct);
	Report.bAllocatedContextCompatible = ClassifyContextStruct(ContextStruct) == EBertaComboGraphEffectContextCompatibility::Compatible;
	Report.Compatibility = Report.bGlobalsClassCompatible && Report.bAllocatedContextCompatible
		? EBertaComboGraphEffectContextCompatibility::Compatible
		: EBertaComboGraphEffectContextCompatibility::Incompatible;
	Report.bCueContainersSafe = Report.Compatibility == EBertaComboGraphEffectContextCompatibility::Compatible;
	Report.Evidence = FString::Printf(
		TEXT("Globals subclass=%s; allocated context derives from FComboGraphGameplayEffectContext=%s."),
		Report.bGlobalsClassCompatible ? TEXT("Yes") : TEXT("No"),
		Report.bAllocatedContextCompatible ? TEXT("Yes") : TEXT("No"));
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
