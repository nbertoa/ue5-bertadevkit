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
	/** Exact match after UE culture resolution. Empty or unresolvable names return false. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Localization")
	static bool IsCurrentLanguage(const FString& Language);

	/** Exact match or a native fallback of the current language: es-AR matches es, but es does not match es-AR. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Localization")
	static bool IsCurrentLanguageCompatibleWith(const FString& Language);

	/** UE's OS-derived default language captured during internationalization initialization, including native fallback. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Localization")
	static FString GetSystemLanguage();

	/** UE's OS-derived default locale captured during internationalization initialization, including native fallback. */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Localization")
	static FString GetSystemLocale();

	/**
	 * Native normalization (en_US becomes en-US), not strict validation or a support check.
	 * Empty stays empty. UE may normalize malformed nonempty names to its invariant culture;
	 * the language predicates use the same native resolution semantics.
	 */
	UFUNCTION(BlueprintPure, Category = "BertaDevKit|Localization")
	static FString CanonicalizeCultureName(const FString& CultureName);
};
