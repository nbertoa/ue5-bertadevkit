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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaLocalizationCanonicalizationTest,
	"BertaDevKit.Localization.Canonicalization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaLocalizationCanonicalizationTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Native canonical separators"), UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("en_US")), FString(TEXT("en-US")));
	TestEqual(TEXT("Native canonical casing"), UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("ES_ar")), FString(TEXT("es-AR")));
	TestTrue(TEXT("Empty input remains empty"), UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("")).IsEmpty());
	TestEqual(TEXT("Malformed input returns the native ICU invariant fallback"),
		UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("!")), FString(TEXT("en-US-POSIX")));
	TestEqual(TEXT("Whitespace returns the native ICU invariant fallback"),
		UBertaLocalizationUtils::CanonicalizeCultureName(TEXT(" \t")), FString(TEXT("en-US-POSIX")));
	TestEqual(TEXT("Normalization does not check culture availability"),
		UBertaLocalizationUtils::CanonicalizeCultureName(TEXT("zz-ZZ")), FString(TEXT("zz-ZZ")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaLocalizationInvalidLanguageNamesTest,
	"BertaDevKit.Localization.Language.InvalidNames",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaLocalizationInvalidLanguageNamesTest::RunTest(const FString& Parameters)
{
	FScopedCultureState Scope;
	FInternationalization& Internationalization = FInternationalization::Get();
	const FString InvariantName = Internationalization.GetInvariantCulture()->GetName();
	if (!TestTrue(TEXT("Invariant language is available"), Internationalization.SetCurrentLanguage(InvariantName)))
	{
		return false;
	}
	const FString InvariantIdentifier = FCulture::CultureNameToVerseIdentifier(InvariantName).ToLower();
	TestTrue(TEXT("Explicit invariant culture remains an exact match"), UBertaLocalizationUtils::IsCurrentLanguage(InvariantName));
	TestTrue(TEXT("Invariant case and separators remain valid"), UBertaLocalizationUtils::IsCurrentLanguage(InvariantIdentifier));
	TestTrue(TEXT("Explicit invariant culture remains compatible"), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(InvariantIdentifier));

	const TCHAR* InvalidNames[] = { TEXT(""), TEXT("!"), TEXT(" \t\r\n"), TEXT("not-a-culture"),
		TEXT("zz-ZZ"), TEXT("C"), TEXT("POSIX"), TEXT("en-US-POSIX!") };
	for (const TCHAR* Query : InvalidNames)
	{
		TestFalse(FString::Printf(TEXT("Invalid query '%s' cannot exactly match invariant"), Query),
			UBertaLocalizationUtils::IsCurrentLanguage(Query));
		TestFalse(FString::Printf(TEXT("Invalid query '%s' cannot match invariant fallback"), Query),
			UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(Query));
	}

	if (!TestTrue(TEXT("Regional English is available"), Internationalization.SetCurrentLanguage(TEXT("en-US"))))
	{
		return false;
	}
	const TCHAR* RepairedNames[] = { TEXT("en-US!"), TEXT(" en-US "), TEXT("e n-US"), TEXT("en--US"), TEXT("en-US-"), TEXT("en!") };
	for (const TCHAR* Query : RepairedNames)
	{
		TestFalse(FString::Printf(TEXT("Sanitized query '%s' cannot exactly match"), Query), UBertaLocalizationUtils::IsCurrentLanguage(Query));
		TestFalse(FString::Printf(TEXT("Sanitized query '%s' cannot match a fallback"), Query), UBertaLocalizationUtils::IsCurrentLanguageCompatibleWith(Query));
	}
	TestTrue(TEXT("Valid case and separators still resolve"), UBertaLocalizationUtils::IsCurrentLanguage(TEXT("EN_us")));
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
	// UE checks the final pair after broadcasting; the listener has already changed the requested locale.
	TestFalse(TEXT("Combined setter reports the reentrant locale differs"), Internationalization.SetCurrentLanguageAndLocale(TEXT("en-US")));
	TestEqual(TEXT("Combined change applies the requested language"), Internationalization.GetCurrentLanguage()->GetName(), FString(TEXT("en-US")));
	TestEqual(TEXT("Reentrant listener applies its locale"), Internationalization.GetCurrentLocale()->GetName(), FString(TEXT("ja")));
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
