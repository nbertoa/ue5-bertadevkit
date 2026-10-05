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
		// Canonicalization also sanitizes and substitutes invalid names. Permit only casing/separator changes,
		// using UE's identifier conversion instead of parsing tags or maintaining a culture alias table.
		if (!FCulture::CultureNameToVerseIdentifier(Language).Equals(
			FCulture::CultureNameToVerseIdentifier(CanonicalName), ESearchCase::IgnoreCase))
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
