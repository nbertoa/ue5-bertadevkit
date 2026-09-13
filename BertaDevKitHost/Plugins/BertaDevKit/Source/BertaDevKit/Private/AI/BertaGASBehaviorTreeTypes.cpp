#include "AI/BertaGASBehaviorTreeTypes.h"

bool FBertaGameplayAttributeCondition::IsSatisfied(const float Value) const
{
	switch (Comparison)
	{
	case EBertaNumericComparison::Less:
		return Value < Threshold;
	case EBertaNumericComparison::LessOrEqual:
		return Value <= Threshold;
	case EBertaNumericComparison::Equal:
		return FMath::IsNearlyEqual(Value, Threshold, FMath::Max(0.0f, Tolerance));
	case EBertaNumericComparison::NotEqual:
		return !FMath::IsNearlyEqual(Value, Threshold, FMath::Max(0.0f, Tolerance));
	case EBertaNumericComparison::GreaterOrEqual:
		return Value >= Threshold;
	case EBertaNumericComparison::Greater:
		return Value > Threshold;
	default:
		return false;
	}
}

FString FBertaGameplayAttributeCondition::ToString() const
{
	const TCHAR* Operator = TEXT("?");
	switch (Comparison)
	{
	case EBertaNumericComparison::Less: Operator = TEXT("<"); break;
	case EBertaNumericComparison::LessOrEqual: Operator = TEXT("<="); break;
	case EBertaNumericComparison::Equal: Operator = TEXT("=="); break;
	case EBertaNumericComparison::NotEqual: Operator = TEXT("!="); break;
	case EBertaNumericComparison::GreaterOrEqual: Operator = TEXT(">="); break;
	case EBertaNumericComparison::Greater: Operator = TEXT(">"); break;
	default: break;
	}

	return FString::Printf(TEXT("%s %s %g"), *Attribute.GetName(), Operator, Threshold);
}
