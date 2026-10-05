#pragma once

#include "Containers/UnrealString.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaLocalizationUtils.generated.h"

/** Small conveniences over UE 5.8 internationalization; language controls text, locale controls formatting. */
UCLASS()
class BERTADEVKIT_API UBertaLocalizationUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Exact match for a native-resolvable name. Only case and '-'/'_' separator normalization is accepted.
	 * Empty, malformed, or rewritten names return false; an explicit invariant culture name remains valid.
	 */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Localization")
	static bool IsCurrentLanguage(const FString& Language);

	/**
	 * Exact match or a native fallback of the current language: es-AR matches es, but es does not match es-AR.
	 * Uses the same input rules as IsCurrentLanguage; malformed names never match invariant culture.
	 */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Localization")
	static bool IsCurrentLanguageCompatibleWith(const FString& Language);

	/**
	 * Native normalization (en_US becomes en-US), not strict validation or a support check.
	 * Empty stays empty. On UE 5.8 with ICU, malformed input such as "!" or whitespace returns en-US-POSIX.
	 * This permissive normalization does not share the language predicates' stricter input rules.
	 */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Localization")
	static FString CanonicalizeCultureName(const FString& CultureName);
};
