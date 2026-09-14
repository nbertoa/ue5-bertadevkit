#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaGameplayTagUsageFinder.generated.h"

UENUM(BlueprintType)
enum class EBertaGameplayTagUsageScope : uint8
{
	SelectedContentBrowserAssets,
	ExplicitGameRoot,
	WholeGame
};

UENUM(BlueprintType)
enum class EBertaGameplayTagMatchMode : uint8
{
	Exact,
	ParentOrChild
};

UENUM(BlueprintType)
enum class EBertaGameplayTagUsageType : uint8
{
	ExactTag,
	Container,
	Query
};

USTRUCT(BlueprintType)
struct BERTADEVKITEDITOR_API FBertaGameplayTagUsageResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Tag Usage")
	FString AssetPath;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Tag Usage")
	FString ObjectPath;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Tag Usage")
	FString ClassPath;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Tag Usage")
	FString PropertyPath;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Tag Usage")
	EBertaGameplayTagUsageType UsageType = EBertaGameplayTagUsageType::ExactTag;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Tag Usage")
	FGameplayTag StoredTag;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay Tag Usage")
	FString Description;
};

/** Read-only, scoped reflection search for stored Gameplay Tag references in project assets. */
UCLASS()
class BERTADEVKITEDITOR_API UBertaGameplayTagUsageFinder final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BertaDevKit|Editor|Gameplay Tags", meta = (ReturnDisplayName = "Success", AdvancedDisplay = "ExplicitGameRoot"))
	static bool FindGameplayTagUsages(
		FGameplayTag Tag,
		EBertaGameplayTagUsageScope Scope,
		EBertaGameplayTagMatchMode MatchMode,
		const FString& ExplicitGameRoot,
		TArray<FBertaGameplayTagUsageResult>& OutResults,
		FString& OutSummary);
};
