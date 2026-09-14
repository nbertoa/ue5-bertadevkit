#pragma once

#include "Abilities/GameplayAbilityTypes.h"
#include "UObject/Object.h"

#include "BertaComboGraphTargetingContext.generated.h"

class UComboGraphNodeAnimBase;

/** Per-request context exposed to Targeting System tasks through FTargetingSourceContext::SourceObject. */
UCLASS(BlueprintType)
class BERTACOMBOGRAPHEXT_API UBertaComboGraphTargetingContext final : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "BertaComboGraphExt|Targeting")
	TObjectPtr<UComboGraphNodeAnimBase> CurrentNode = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "BertaComboGraphExt|Targeting")
	FGameplayTag ContainerEventTag;

	UPROPERTY(BlueprintReadOnly, Category = "BertaComboGraphExt|Targeting")
	FGameplayEventData EventData;
};
