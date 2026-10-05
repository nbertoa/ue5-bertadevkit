#include "Localization/BertaLocalizationUtils.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"

namespace
{
	FCulturePtr ResolveLanguageQuery(FInternationalization& Internationalization, const FString& Language)
	{
		if (Language.IsEmpty())
		{
			return nullptr;
		}

		const FString CanonicalName = FCulture::GetCanonicalName(Language);
		FString NormalizedInput = Language;
		NormalizedInput.ReplaceCharInline(TEXT('_'), TEXT('-'), ESearchCase::CaseSensitive);
		// Canonicalization also sanitizes and substitutes invalid names. Permit only casing/separator changes,
		// without parsing tags or requiring the arbitrary input to already be a well-formed culture name.
		if (!NormalizedInput.Equals(CanonicalName, ESearchCase::IgnoreCase))
		{
			return nullptr;
		}
		return Internationalization.GetCulture(CanonicalName);
	}
}

bool UBertaLocalizationUtils::IsCurrentLanguage(const FString& Language)
{
	FInternationalization& Internationalization = FInternationalization::Get();
	const FCulturePtr Query = ResolveLanguageQuery(Internationalization, Language);
	return Query.IsValid() && Query->GetName() == Internationalization.GetCurrentLanguage()->GetName();
}

bool UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(const FString& Language)
{
	FInternationalization& Internationalization = FInternationalization::Get();
	const FCulturePtr Query = ResolveLanguageQuery(Internationalization, Language);
	if (!Query.IsValid())
	{
		return false;
	}

	const FString& CurrentLanguage = Internationalization.GetCurrentLanguage()->GetName();
	// The exact check also covers variants/custom cultures absent from a parent fallback chain.
	return Query->GetName() == CurrentLanguage
		|| Internationalization.GetPrioritizedCultureNames(CurrentLanguage).Contains(Query->GetName());
}

FString UBertaLocalizationUtils::CanonicalizeCultureName(const FString& CultureName)
{
	return CultureName.IsEmpty() ? FString() : FCulture::GetCanonicalName(CultureName);
}
