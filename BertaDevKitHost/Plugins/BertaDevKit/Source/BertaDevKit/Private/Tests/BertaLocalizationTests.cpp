#include "Localization/BertaLocalizationUtils.h"

#include "Tests/BertaLocalizationTestReceiver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Localization/BertaLocalizationSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/SubsystemCollection.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/NameTypes.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	struct FScopedCultureState
	{
		FScopedCultureState()
		{
			FInternationalization::Get().BackupCultureState(Snapshot);
		}

		~FScopedCultureState()
		{
			FInternationalization::Get().RestoreCultureState(Snapshot);
		}

		FInternationalization::FCultureStateSnapshot Snapshot;
	};

	struct FScopedLocalizationListener
	{
		FScopedLocalizationListener()
			: Subsystem(NewObject<UBertaLocalizationSubsystem>(GEngine))
			, Receiver(NewObject<UBertaLocalizationTestReceiver>())
		{
			Subsystem->OnLanguageChanged.AddDynamic(Receiver.Get(), &UBertaLocalizationTestReceiver::RecordLanguage);
			Subsystem->OnLocaleChanged.AddDynamic(Receiver.Get(), &UBertaLocalizationTestReceiver::RecordLocale);
			Subsystem->Initialize(Collection);
		}

		~FScopedLocalizationListener()
		{
			if (bInitialized)
			{
				Subsystem->Deinitialize();
			}
		}

		FSubsystemCollection<UEngineSubsystem> Collection;
		TStrongObjectPtr<UBertaLocalizationSubsystem> Subsystem;
		TStrongObjectPtr<UBertaLocalizationTestReceiver> Receiver;
		bool bInitialized = true;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaLocalizationExactLanguageTest,
	"BertaDevKit.Localization.Language.Exact",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaLocalizationExactLanguageTest::RunTest(const FString& Parameters)
{
	FScopedCultureState Scope;
	FInternationalization& Internationalization = FInternationalization::Get();
	if (!TestTrue(TEXT("English is available"), Internationalization.SetCurrentLanguage(TEXT("en"))))
	{
		return false;
	}
	TestTrue(TEXT("Exact base language"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("en")));
	TestFalse(TEXT("Different language"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("fr")));
	TestFalse(TEXT("Empty query"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("")));
	TestFalse(TEXT("Unresolvable query"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("zz-ZZ")));
	if (!TestTrue(TEXT("Regional English is available"), Internationalization.SetCurrentLanguage(TEXT("en-US"))))
	{
		return false;
	}
	TestTrue(TEXT("Exact regional language"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("en-US")));
	TestTrue(TEXT("Native case and separator normalization"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("EN_us")));
	TestFalse(TEXT("A parent is not an exact match"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("en")));
	TestTrue(TEXT("Independent locale is available"), Internationalization.SetCurrentLocale(TEXT("fr")));
	TestFalse(TEXT("Locale does not determine text language"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("fr")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaLocalizationFallbackLanguageTest,
	"BertaDevKit.Localization.Language.Fallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaLocalizationFallbackLanguageTest::RunTest(const FString& Parameters)
{
	FScopedCultureState Scope;
	FInternationalization& Internationalization = FInternationalization::Get();
	if (!TestTrue(TEXT("Argentinian Spanish is available"), Internationalization.SetCurrentLanguage(TEXT("es-AR"))))
	{
		return false;
	}
	TestTrue(TEXT("Exact culture is compatible"), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(TEXT("ES_ar")));
	TestTrue(TEXT("Parent language is a native fallback"), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(TEXT("es")));
	TestFalse(TEXT("Sibling region is not a fallback"), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(TEXT("es-ES")));
	TestFalse(TEXT("Different language is not a fallback"), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(TEXT("en")));
	TestFalse(TEXT("Empty query cannot match a fallback"), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(TEXT("")));
	TestFalse(TEXT("Unknown query must not acquire the default English fallback"), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(TEXT("zz-ZZ")));
	if (!TestTrue(TEXT("Base Spanish is available"), Internationalization.SetCurrentLanguage(TEXT("es"))))
	{
		return false;
	}
	TestFalse(TEXT("Compatibility is directional"), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(TEXT("es-AR")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaLocalizationNormalizationAndDefaultsTest,
	"BertaDevKit.Localization.NormalizationAndDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaLocalizationNormalizationAndDefaultsTest::RunTest(const FString& Parameters)
{
	FScopedCultureState Scope;
	FInternationalization& Internationalization = FInternationalization::Get();
	const FString DefaultLanguage = Internationalization.GetDefaultLanguage()->GetName();
	const FString DefaultLocale = Internationalization.GetDefaultLocale()->GetName();
	TestEqual(TEXT("Native canonical separators"), UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("en_US")), FString(TEXT("en-US")));
	TestEqual(TEXT("Native canonical casing"), UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("ES_ar")), FString(TEXT("es-AR")));
	TestTrue(TEXT("Empty input remains empty"), UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("")).IsEmpty());
	TestEqual(TEXT("Malformed input preserves UE normalization fallback"), UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("!")), FCulture::GetCanonicalName(TEXT("!")));
	if (!TestTrue(TEXT("Invariant language is available"), Internationalization.SetCurrentLanguage(Internationalization.GetInvariantCulture()->GetName())))
	{
		return false;
	}
	TestTrue(TEXT("Malformed names resolving to the invariant culture can match it"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("!")));
	TestFalse(TEXT("Empty is rejected even when UE would resolve it to invariant"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("")));
	if (!TestTrue(TEXT("Independent language and locale are available"), Internationalization.SetCurrentLanguage(TEXT("fr")))
		|| !TestTrue(TEXT("Locale is available"), Internationalization.SetCurrentLocale(TEXT("de"))))
	{
		return false;
	}
	TestEqual(TEXT("System language remains the UE startup default"), UBertaLocalizationUtils::GetSystemLanguage(), DefaultLanguage);
	TestEqual(TEXT("System locale remains the UE startup default"), UBertaLocalizationUtils::GetSystemLocale(), DefaultLocale);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaLocalizationNotificationsTest,
	"BertaDevKit.Localization.Subsystem.Notifications",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaLocalizationNotificationsTest::RunTest(const FString& Parameters)
{
	FScopedCultureState Scope;
	FInternationalization& Internationalization = FInternationalization::Get();
	if (!TestTrue(TEXT("Initial language is available"), Internationalization.SetCurrentLanguage(TEXT("en")))
		|| !TestTrue(TEXT("Initial locale is available"), Internationalization.SetCurrentLocale(TEXT("fr"))))
	{
		return false;
	}

	FScopedLocalizationListener Listener;
	TArray<FString>& Events = Listener.Receiver->Events;
	TestEqual(TEXT("Initialization emits no events"), Events.Num(), 0);
	TestTrue(TEXT("Native locale change"), Internationalization.SetCurrentLocale(TEXT("de")));
	TestTrue(TEXT("Only locale changed with correct payload"), Events == TArray<FString>{TEXT("Locale:fr->de")});
	Events.Reset();
	TestTrue(TEXT("Native language change"), Internationalization.SetCurrentLanguage(TEXT("es-AR")));
	TestTrue(TEXT("Only language changed with correct payload"), Events == TArray<FString>{TEXT("Language:en->es-AR")});
	Events.Reset();
	TestTrue(TEXT("Native unchanged language"), Internationalization.SetCurrentLanguage(TEXT("ES_ar")));
	TestTrue(TEXT("Native unchanged locale"), Internationalization.SetCurrentLocale(TEXT("de")));
	TestTrue(TEXT("Native asset group change"), Internationalization.SetCurrentAssetGroupCulture(TEXT("BertaLocalizationAutomation"), TEXT("ja")));
	TestEqual(TEXT("Unchanged values and asset groups emit no language/locale events"), Events.Num(), 0);

	Listener.Receiver->ReentrantLocale = TEXT("ja");
	TestTrue(TEXT("Native combined change"), Internationalization.SetCurrentLanguageAndLocale(TEXT("en-US")));
	TestTrue(TEXT("A listener changing locale preserves transition order and payloads"), Events == TArray<FString>{
		TEXT("Language:es-AR->en-US"), TEXT("Locale:de->en-US"), TEXT("Locale:en-US->ja")});
	Events.Reset();
	Listener.Subsystem->Deinitialize();
	Listener.bInitialized = false;
	TestTrue(TEXT("Native change after unregister"), Internationalization.SetCurrentLanguageAndLocale(TEXT("fr")));
	TestEqual(TEXT("Deinitialize removes the listener"), Events.Num(), 0);
	return true;
}

#endif

void UBertaLocalizationTestReceiver::RecordLanguage(const FString& PreviousLanguage, const FString& CurrentLanguage)
{
#if WITH_DEV_AUTOMATION_TESTS
	Events.Add(FString::Printf(TEXT("Language:%s->%s"), *PreviousLanguage, *CurrentLanguage));
	if (!ReentrantLocale.IsEmpty())
	{
		const FString NextLocale = MoveTemp(ReentrantLocale);
		FInternationalization::Get().SetCurrentLocale(NextLocale);
	}
#endif
}

void UBertaLocalizationTestReceiver::RecordLocale(const FString& PreviousLocale, const FString& CurrentLocale)
{
#if WITH_DEV_AUTOMATION_TESTS
	Events.Add(FString::Printf(TEXT("Locale:%s->%s"), *PreviousLocale, *CurrentLocale));
#endif
}
