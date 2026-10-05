#include "Localization/BertaLocalizationUtils.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"

bool UBertaLocalizationUtils::IsCurrentLanguage(const FString& Language)
{
	if (Language.IsEmpty())
	{
		return false;
	}

	FInternationalization& Internationalization = FInternationalization::Get();
	const FCulturePtr Query = Internationalization.GetCulture(Language);
	return Query.IsValid() && Query->GetName() == Internationalization.GetCurrentLanguage()->GetName();
}

bool UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(const FString& Language)
{
	if (Language.IsEmpty())
	{
		return false;
	}

	FInternationalization& Internationalization = FInternationalization::Get();
	const FCulturePtr Query = Internationalization.GetCulture(Language);
	if (!Query.IsValid())
	{
		return false;
	}

	const FString& CurrentLanguage = Internationalization.GetCurrentLanguage()->GetName();
	// The exact check also covers variants/custom cultures absent from a parent fallback chain.
	return Query->GetName() == CurrentLanguage
		|| Internationalization.GetPrioritizedCultureNames(CurrentLanguage).Contains(Query->GetName());
}

FString UBertaLocalizationUtils::GetSystemLanguage()
{
	return FInternationalization::Get().GetDefaultLanguage()->GetName();
}

FString UBertaLocalizationUtils::GetSystemLocale()
{
	return FInternationalization::Get().GetDefaultLocale()->GetName();
}

FString UBertaLocalizationUtils::CanonicalizeCultureName(const FString& CultureName)
{
	return CultureName.IsEmpty() ? FString() : FCulture::GetCanonicalName(CultureName);
}
