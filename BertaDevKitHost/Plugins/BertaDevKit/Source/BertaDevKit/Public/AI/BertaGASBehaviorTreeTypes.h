#pragma once

#include "AttributeSet.h"
#include "CoreMinimal.h"

#include "BertaGASBehaviorTreeTypes.generated.h"

UENUM(BlueprintType)
enum class EBertaNumericComparison : uint8
{
	Less,
	LessOrEqual UMETA(DisplayName = "Less Or Equal"),
	Equal,
	NotEqual UMETA(DisplayName = "Not Equal"),
	GreaterOrEqual UMETA(DisplayName = "Greater Or Equal"),
	Greater
};

USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaGameplayAttributeCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Attribute")
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Attribute")
	EBertaNumericComparison Comparison = EBertaNumericComparison::Less;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Attribute")
	float Threshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Attribute", meta = (ClampMin = "0.0"))
	float Tolerance = KINDA_SMALL_NUMBER;

	bool IsSatisfied(float Value) const;
	FString ToString() const;
};
