#include "AssetActions/BertaAssetAuditor.h"

#include "AssetActions/BertaAssetNamingUtils.h"
#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityLibrary.h"
#include "Engine/Blueprint.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

// ----------------------------------------------------------------
// Internal helpers
// ----------------------------------------------------------------

namespace
{
	/** Builds and displays an FNotificationInfo toast with the given message. */
	void ShowNotification(const FText& Message,
	                      const SNotificationItem::ECompletionState State)
	{
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 4.0f;

		TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);

		if (Notification.IsValid())
		{
			Notification->SetCompletionState(State);
		}
	}

	/**
	 * Prefix map for classes from optional plugins that cannot be referenced
	 * via StaticClass() without creating a hard module dependency.
	 *
	 * Keyed by native class name (FName). These names are stable across UE
	 * versions — Epic cannot rename them without breaking backward compatibility
	 * for thousands of existing projects.
	 */
	const TMap<FName, FString>& GetOptionalPluginPrefixes()
	{
		static const TMap<FName, FString> OptionalPrefixes = {
			// Gameplay Ability System — requires the GameplayAbilities plugin.
			// Listed here to avoid a hard dependency that prevents editor launch
			// on projects where the plugin is disabled.
			{FName(TEXT("GameplayAbility")), TEXT("GA_")}, {FName(TEXT("GameplayEffect")), TEXT("GE_")},
			{FName(TEXT("GameplayCueNotify_Static")), TEXT("GC_")},
			{FName(TEXT("GameplayCueNotify_Actor")), TEXT("GC_")},
		};

		return OptionalPrefixes;
	}
}

// ----------------------------------------------------------------
// FindPrefixForClass
// ----------------------------------------------------------------

const FString* UBertaAssetAuditor::FindPrefixForClass(UClass* AssetClass,
                                                      UObject* Asset)
{
	const TMap<UClass*, FString>& PrefixMap = UBertaAssetNamingUtils::GetPrefixMap();
	const TMap<FName, FString>& OptionalPrefixes = GetOptionalPluginPrefixes();

	// For Blueprint assets, the asset class is always UBlueprint (or a subclass like
	// UAnimBlueprint). The *meaningful* class for prefix resolution is the native parent
	// that the Blueprint derives from — stored in UBlueprint::ParentClass.
	//
	// Example: a Blueprint of GameMode has AssetClass = UBlueprint, but
	// ParentClass walks through AGameModeBase, which carries the "GM_" prefix.
	//
	// This path also covers GAS Blueprints (GA_, GE_) without requiring the
	// GameplayAbilities module — we compare by class name string instead of StaticClass().
	if (const UBlueprint* const BP = Cast<UBlueprint>(Asset))
	{
		UClass* ParentClass = BP->ParentClass;

		while (ParentClass)
		{
			// Check the main prefix map first (framework classes: GameMode, PlayerController, etc.).
			if (const FString* Found = PrefixMap.Find(ParentClass))
			{
				return Found;
			}

			// Check the optional plugin map by class name to avoid hard dependencies.
			if (const FString* Found = OptionalPrefixes.Find(ParentClass->GetFName()))
			{
				return Found;
			}

			ParentClass = ParentClass->GetSuperClass();
		}
	}

	// Non-Blueprint assets: walk the asset's own class hierarchy directly.
	UClass* CurrentClass = AssetClass;

	while (CurrentClass)
	{
		if (const FString* Found = PrefixMap.Find(CurrentClass))
		{
			return Found;
		}

		CurrentClass = CurrentClass->GetSuperClass();
	}

	return nullptr;
}

// ----------------------------------------------------------------
// ResolveAssetScope
// ----------------------------------------------------------------

void UBertaAssetAuditor::ResolveAssetScope(TArray<FAssetData>& OutAssets)
{
	OutAssets.Reset();

	// Priority 1: individually selected assets in the Content Browser.
	// This gives the user the most fine-grained control — process exactly what is selected.
	const TArray<FAssetData> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssetData();

	if (!SelectedAssets.IsEmpty())
	{
		OutAssets = SelectedAssets;

		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[UBertaAssetAuditor::ResolveAssetScope] Using %d selected asset(s)."),
		       OutAssets.Num());

		return;
	}

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;

	// Priority 2: the folder active in the Content Browser directory tree.
	// GetCurrentContentBrowserPath returns the path the user navigated to on the
	// left panel — this is what "run on this folder" means in practice.
	FString ActiveFolderPath;
	const bool bHasActivePath = UEditorUtilityLibrary::GetCurrentContentBrowserPath(ActiveFolderPath);

	if (bHasActivePath && !ActiveFolderPath.IsEmpty())
	{
		Filter.PackagePaths.Add(FName(*ActiveFolderPath));

		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[UBertaAssetAuditor::ResolveAssetScope] Scanning active folder: %s"),
		       *ActiveFolderPath);
	}
	else
	{
		// Priority 3: no selection and no active folder — full /Game/ scan as last resort.
		Filter.PackagePaths.Add(FName(TEXT("/Game")));

		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[UBertaAssetAuditor::ResolveAssetScope] No folder active — falling back to full /Game/ scan."));
	}

	AssetRegistryModule.Get().GetAssets(Filter,
	                                    OutAssets);

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::ResolveAssetScope] Found %d asset(s) in scope."),
	       OutAssets.Num());
}

// ----------------------------------------------------------------
// RunAudit
// ----------------------------------------------------------------

void UBertaAssetAuditor::RunAudit()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::RunAudit] Starting audit..."));

	TArray<FAssetData> Assets;
	ResolveAssetScope(Assets);

	int32 ViolationCount = 0;

	for (const FAssetData& AssetData : Assets)
	{
		UClass* AssetClass = AssetData.GetClass();

		if (!AssetClass)
		{
			// Class not loaded — skip silently, not a violation.
			continue;
		}

		// Pass nullptr as Asset — RunAudit does not load assets into memory.
		// FindPrefixForClass handles the null Asset case by skipping the Blueprint
		// ParentClass walk and falling through to the direct class hierarchy walk.
		const FString* FoundPrefix = FindPrefixForClass(AssetClass,
		                                                nullptr);

		if (!FoundPrefix || FoundPrefix->IsEmpty())
		{
			UE_LOG(LogBertaDevKitEditor,
			       Verbose,
			       TEXT("[UBertaAssetAuditor::RunAudit] Unknown class, skipping: %s (%s)"),
			       *AssetData.AssetName.ToString(),
			       *AssetData.AssetClassPath.GetAssetName().ToString());
			continue;
		}

		const FString AssetName = AssetData.AssetName.ToString();

		if (!AssetName.StartsWith(*FoundPrefix))
		{
			UE_LOG(LogBertaDevKitEditor,
			       Warning,
			       TEXT(
				       "[UBertaAssetAuditor::RunAudit] VIOLATION — Asset: %s | Class: %s | Expected prefix: %s | Path: %s"
			       ),
			       *AssetName,
			       *AssetClass->GetName(),
			       **FoundPrefix,
			       *AssetData.PackagePath.ToString());

			++ViolationCount;
		}
	}

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::RunAudit] Audit complete — %d violation(s) found."),
	       ViolationCount);

	const FText NotificationText = ViolationCount > 0
		                               ? FText::Format(NSLOCTEXT("BertaDevKit",
		                                                         "AuditViolations",
		                                                         "Asset Audit: {0} violation(s) found. See Output Log."),
		                                               FText::AsNumber(ViolationCount))
		                               : NSLOCTEXT("BertaDevKit",
		                                           "AuditClean",
		                                           "Asset Audit: No violations found.");

	const SNotificationItem::ECompletionState NotificationState = ViolationCount > 0
		                                                              ? SNotificationItem::CS_Fail
		                                                              : SNotificationItem::CS_Success;

	ShowNotification(NotificationText,
	                 NotificationState);
}

// ----------------------------------------------------------------
// RunAuditAndFix
// ----------------------------------------------------------------

void UBertaAssetAuditor::RunAuditAndFix()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::RunAuditAndFix] Starting audit and fix..."));

	TArray<FAssetData> Assets;
	ResolveAssetScope(Assets);

	int32 RenamedCount = 0;
	int32 SkippedCount = 0;

	for (const FAssetData& AssetData : Assets)
	{
		UClass* AssetClass = AssetData.GetClass();

		if (!AssetClass)
		{
			continue;
		}

		// Load the asset first — FindPrefixForClass needs the UBlueprint object
		// to inspect ParentClass for framework and optional-plugin Blueprints.
		UObject* const LoadedAsset = AssetData.GetAsset();

		if (!IsValid(LoadedAsset))
		{
			UE_LOG(LogBertaDevKitEditor,
			       Warning,
			       TEXT("[UBertaAssetAuditor::RunAuditAndFix] Failed to load asset: %s"),
			       *AssetData.AssetName.ToString());
			++SkippedCount;
			continue;
		}

		const FString* FoundPrefix = FindPrefixForClass(AssetClass,
		                                                LoadedAsset);

		if (!FoundPrefix || FoundPrefix->IsEmpty())
		{
			// Unknown class — skip silently.
			UE_LOG(LogBertaDevKitEditor,
			       Verbose,
			       TEXT("[UBertaAssetAuditor::RunAuditAndFix] Unknown class, skipping: %s"),
			       *AssetData.AssetName.ToString());
			continue;
		}

		// Asset already correct — nothing to fix.
		if (AssetData.AssetName.ToString().StartsWith(*FoundPrefix))
		{
			continue;
		}

		const EBertaRenameResult Result = UBertaAssetNamingUtils::RenameAssetWithPrefix(LoadedAsset);

		if (Result == EBertaRenameResult::Renamed)
		{
			UE_LOG(LogBertaDevKitEditor,
			       Log,
			       TEXT("[UBertaAssetAuditor::RunAuditAndFix] Fixed: %s"),
			       *AssetData.AssetName.ToString());
			++RenamedCount;
		}
		else
		{
			// Should not happen — we pre-checked the prefix and class above.
			// Log as warning to catch any edge cases.
			UE_LOG(LogBertaDevKitEditor,
			       Warning,
			       TEXT("[UBertaAssetAuditor::RunAuditAndFix] Unexpected result for: %s"),
			       *AssetData.AssetName.ToString());
			++SkippedCount;
		}
	}

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::RunAuditAndFix] Done — renamed %d asset(s), skipped %d."),
	       RenamedCount,
	       SkippedCount);

	const FText NotificationText = FText::Format(NSLOCTEXT("BertaDevKit",
	                                                       "AuditFixDone",
	                                                       "Asset Fix: {0} renamed, {1} skipped. See Output Log."),
	                                             FText::AsNumber(RenamedCount),
	                                             FText::AsNumber(SkippedCount));

	const SNotificationItem::ECompletionState NotificationState = SkippedCount > 0
		                                                              ? SNotificationItem::CS_Fail
		                                                              : SNotificationItem::CS_Success;

	ShowNotification(NotificationText,
	                 NotificationState);
}
