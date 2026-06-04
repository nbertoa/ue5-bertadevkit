#include "AssetActions/BertaAssetAuditor.h"

#include "AssetActions/BertaAssetNamingUtils.h"
#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityLibrary.h"
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
}

// ----------------------------------------------------------------
// FindPrefixForClass
// ----------------------------------------------------------------

const FString* UBertaAssetAuditor::FindPrefixForClass(UClass* AssetClass)
{
	const TMap<UClass*, FString>& PrefixMap = UBertaAssetNamingUtils::GetPrefixMap();

	// Walk up the hierarchy to find the nearest registered prefix.
	// This correctly handles Blueprint subclasses of known types (e.g. a BP
	// subclassing UDataAsset matches UDataAsset's "DA_" entry).
	UClass* CurrentClass = AssetClass;

	while (CurrentClass)
	{
		const FString* Found = PrefixMap.Find(CurrentClass);

		if (Found)
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

	// Prefer the current Content Browser selection — gives the user fine-grained control.
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

	// No selection — fall back to a full /Game/ scan via the Asset Registry.
	// Assets are not loaded into memory; only their metadata is queried.
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game"));
	Filter.bRecursivePaths = true;

	AssetRegistryModule.Get().GetAssets(Filter,
	                                    OutAssets);

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::ResolveAssetScope] Full scan — found %d asset(s) under /Game/."),
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

		const FString* FoundPrefix = FindPrefixForClass(AssetClass);

		if (!FoundPrefix || FoundPrefix->IsEmpty())
		{
			// Unknown class — log at Verbose and skip. Not a violation.
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

		const FString* FoundPrefix = FindPrefixForClass(AssetClass);

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

		// Load the asset into memory only when we know it needs fixing.
		// Avoids loading the entire project for a full /Game/ scan.
		UObject* const LoadedAsset = AssetData.GetAsset();

		if (!IsValid(LoadedAsset))
		{
			UE_LOG(LogBertaDevKitEditor,
			       Warning,
			       TEXT("[UBertaAssetAuditor::RunAuditAndFix] Failed to load asset for fixing: %s"),
			       *AssetData.AssetName.ToString());
			++SkippedCount;
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
			// Should not happen — we pre-checked the prefix above.
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

	// Show failure state if any assets could not be loaded and processed.
	const SNotificationItem::ECompletionState NotificationState = SkippedCount > 0
		                                                              ? SNotificationItem::CS_Fail
		                                                              : SNotificationItem::CS_Success;

	ShowNotification(NotificationText,
	                 NotificationState);
}
