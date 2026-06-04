#include "AssetActions/BertaAssetAuditor.h"

#include "AssetActions/BertaAssetNamingUtils.h"
#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityLibrary.h"
#include "Engine/Blueprint.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Materials/MaterialInstanceConstant.h"
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
			{FName(TEXT("GameplayAbilityBlueprint")), TEXT("GA_")},
			{FName(TEXT("GameplayEffectBlueprint")), TEXT("GE_")},
		};

		return OptionalPrefixes;
	}

	/**
	 * Extracts the native class name from an Asset Registry "ParentClass" tag value.
	 *
	 * Expected format: /Script/CoreUObject.Class'/Script/Module.ClassName'
	 * Returns the ClassName portion after the last '.', or NAME_None if the
	 * format is unexpected.
	 */
	FName ExtractClassNameFromTag(const FString& TagValue)
	{
		// Find the last '.' — the class name follows it.
		int32 DotIndex = INDEX_NONE;
		if (!TagValue.FindLastChar(TEXT('.'),
		                           DotIndex))
		{
			return NAME_None;
		}

		// Extract everything after the last '.' and strip the closing '\''.
		FString ClassName = TagValue.RightChop(DotIndex + 1);
		ClassName.RemoveFromEnd(TEXT("'"));

		if (ClassName.IsEmpty())
		{
			return NAME_None;
		}

		return FName(*ClassName);
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

	// Path A: asset is loaded and is a Blueprint — walk UBlueprint::ParentClass directly.
	// The meaningful class for prefix resolution is the native parent, not the asset
	// class itself (which is always UBlueprint or a subclass like UAnimBlueprint).
	if (const UBlueprint* const BP = Cast<UBlueprint>(Asset))
	{
		UClass* ParentClass = BP->ParentClass;

		while (ParentClass)
		{
			if (const FString* Found = PrefixMap.Find(ParentClass))
			{
				return Found;
			}

			if (const FString* Found = OptionalPrefixes.Find(ParentClass->GetFName()))
			{
				return Found;
			}

			ParentClass = ParentClass->GetSuperClass();
		}

		// Blueprint with no registered parent — unknown.
		return nullptr;
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
// ResolveBlueprintPrefixFromTag
// ----------------------------------------------------------------

const FString* UBertaAssetAuditor::ResolveBlueprintPrefixFromTag(const FAssetData& AssetData)
{
	const TMap<UClass*, FString>& PrefixMap = UBertaAssetNamingUtils::GetPrefixMap();
	const TMap<FName, FString>& OptionalPrefixes = GetOptionalPluginPrefixes();

	UE_LOG(LogBertaDevKitEditor,
	       Warning,
	       TEXT("[DEBUG] ResolveBlueprintPrefixFromTag — Asset: %s"),
	       *AssetData.AssetName.ToString());

	// If the asset class itself has a direct entry in the prefix map
	// (e.g. UAnimBlueprint → "ABP_"), use it immediately without parsing the tag.
	// This avoids misrouting AnimBlueprints through the ParentClass walk,
	// which would resolve to the animation class rather than UAnimBlueprint.
	UClass* const AssetClass = AssetData.GetClass();
	if (AssetClass)
	{
		// For UBlueprint specifically, skip the direct map lookup and proceed to the
		// ParentClass tag walk — the meaningful prefix lives in the native parent hierarchy,
		// not in the asset class itself.
		// Specific Blueprint subclasses (UAnimBlueprint, UGameplayAbilityBlueprint, etc.)
		// are still resolved here via the direct lookup.
		const bool bIsGenericBlueprint = (AssetClass == UBlueprint::StaticClass());

		if (!bIsGenericBlueprint)
		{
			if (const FString* Found = PrefixMap.Find(AssetClass))
			{
				return Found;
			}

			if (const FString* Found = OptionalPrefixes.Find(AssetClass->GetFName()))
			{
				return Found;
			}
		}
	}

	// Read the "ParentClass" tag stored by the Asset Registry.
	// This tag is populated for all Blueprint assets without loading them into memory.
	// Format: /Script/CoreUObject.Class'/Script/Module.ClassName'
	FString ParentClassTag;
	AssetData.GetTagValue(TEXT("ParentClass"),
	                      ParentClassTag);

	if (ParentClassTag.IsEmpty())
	{
		// No tag — fall back to generic BP_ prefix.
		return PrefixMap.Find(UBlueprint::StaticClass());
	}

	const FName ParentClassName = ExtractClassNameFromTag(ParentClassTag);

	if (ParentClassName == NAME_None)
	{
		return PrefixMap.Find(UBlueprint::StaticClass());
	}

	// Check the optional plugin map first (GAS, etc.) — keyed by class name string.
	if (const FString* Found = OptionalPrefixes.Find(ParentClassName))
	{
		return Found;
	}

	// Try to resolve the actual UClass and walk the main prefix map.
	// This covers framework classes (AGameModeBase, APlayerController, etc.)
	// that are always available without optional plugins.
	if (UClass* const ParentClass = FindFirstObject<UClass>(*ParentClassName.ToString(),
	                                                        EFindFirstObjectOptions::None))
	{
		UClass* Current = ParentClass;

		while (Current)
		{
			if (const FString* Found = PrefixMap.Find(Current))
			{
				return Found;
			}

			Current = Current->GetSuperClass();
		}
	}

	// Parent class not registered — fall back to generic BP_ prefix.
	return PrefixMap.Find(UBlueprint::StaticClass());
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

		// For Blueprint assets, resolve the prefix via the Asset Registry tag to
		// avoid loading the asset into memory during a dry audit.
		const FString* FoundPrefix = nullptr;

		if (AssetClass == UBlueprint::StaticClass() || AssetClass->IsChildOf(UBlueprint::StaticClass()))
		{
			FoundPrefix = ResolveBlueprintPrefixFromTag(AssetData);
		}
		else
		{
			// Non-Blueprint: walk the asset class hierarchy directly.
			// Asset is not needed — pass nullptr.
			FoundPrefix = FindPrefixForClass(AssetClass,
			                                 nullptr);
		}

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

		// Use FindPrefixForClass for loaded Blueprint assets so the ParentClass walk
		// is used — same path as RunAudit. For generic Blueprints where ParentClass
		// walk fails, fall back to ResolveBlueprintPrefixFromTag.
		const FString* FoundPrefix = nullptr;

		if (AssetClass == UBlueprint::StaticClass() || AssetClass->IsChildOf(UBlueprint::StaticClass()))
		{
			FoundPrefix = FindPrefixForClass(AssetClass,
			                                 LoadedAsset);

			// FindPrefixForClass returns nullptr when the ParentClass walk finds no match.
			// Fall back to the tag-based resolver which covers cases where ParentClass
			// is not yet loaded but the Asset Registry tag is available.
			if (!FoundPrefix)
			{
				FoundPrefix = ResolveBlueprintPrefixFromTag(AssetData);
			}
		}
		else
		{
			FoundPrefix = FindPrefixForClass(AssetClass,
			                                 nullptr);
		}

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

		// Rename using the resolved prefix directly — bypasses RenameAssetWithPrefix's
		// own class walk, which does not have access to ParentClass context.
		FString CleanName = AssetData.AssetName.ToString();

		// Material instances auto-named by the engine may carry "M_" and "_Inst".
		if (LoadedAsset->IsA<UMaterialInstanceConstant>())
		{
			CleanName.RemoveFromStart(TEXT("M_"));
			CleanName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewName = *FoundPrefix + CleanName;
		UEditorUtilityLibrary::RenameAsset(LoadedAsset,
		                                   NewName);

		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[UBertaAssetAuditor::RunAuditAndFix] Fixed: %s → %s"),
		       *AssetData.AssetName.ToString(),
		       *NewName);

		++RenamedCount;
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
