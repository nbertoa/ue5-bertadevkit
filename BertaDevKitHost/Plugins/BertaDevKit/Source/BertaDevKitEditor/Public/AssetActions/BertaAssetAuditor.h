#pragma once

#include "UObject/Object.h"

#include "BertaAssetAuditor.generated.h"

/**
 * Scans assets in the project and reports or fixes naming convention violations.
 *
 * Scope resolution priority:
 *   1. Individually selected assets in the Content Browser.
 *   2. The folder currently active in the Content Browser directory tree.
 *   3. Full recursive scan under /Game/ as a last resort.
 *
 * @note This class is never instantiated — all entry points are static.
 */
UCLASS()
class BERTADEVKITEDITOR_API UBertaAssetAuditor : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Scans assets and reports naming violations to LogBertaDevKitEditor.
	 * Displays an FNotificationInfo with the total violation count on completion.
	 * Does not modify any assets.
	 */
	static void RunAudit();

	/**
	 * Scans assets for naming violations and renames each violator using
	 * UBertaAssetNamingUtils::RenameAssetWithPrefix().
	 * Reports renamed and failed counts via LogBertaDevKitEditor and FNotificationInfo.
	 */
	static void RunAuditAndFix();

private:
	/**
	 * Collects the asset scope using the following priority:
	 *   1. Selected assets — returned immediately if any are selected.
	 *   2. Active Content Browser folder — scanned recursively via Asset Registry.
	 *   3. Full /Game/ scan — used only when neither of the above yields a scope.
	 *
	 * @param OutAssets  Populated with the resolved asset scope.
	 */
	static void ResolveAssetScope(TArray<FAssetData>& OutAssets);

	/**
	 * Resolves the expected name prefix for a given asset.
	 *
	 * For Blueprint assets, walks the native parent class hierarchy (via UBlueprint::ParentClass)
	 * to correctly identify framework subclasses (e.g. GameMode, PlayerController) and
	 * optional-plugin classes (e.g. GameplayAbility, GameplayEffect) without requiring
	 * a hard module dependency on GameplayAbilities.
	 *
	 * For non-Blueprint assets, walks the asset's own class hierarchy.
	 *
	 * @param AssetClass  The class of the asset as returned by FAssetData::GetClass(). Must not be null.
	 * @param Asset       The asset UObject. Used to inspect UBlueprint::ParentClass when applicable.
	 *                    May be null for audit-only paths where the asset is not loaded.
	 * @return            Pointer to the registered prefix string, or nullptr if unknown.
	 */
	static const FString* FindPrefixForClass(UClass* AssetClass,
	                                         UObject* Asset);

	/**
	 * Resolves the expected prefix for a Blueprint asset without loading it into memory.
	 * Reads the "ParentClass" Asset Registry tag and walks the class hierarchy via
	 * ExtractClassNameFromTag, checking both the main prefix map and the optional
	 * plugin map (GAS, etc.) by class name string.
	 *
	 * Falls back to "BP_" if the tag is absent or the parent class is not registered.
	 *
	 * @param AssetData  The asset metadata from the Asset Registry.
	 * @return           Pointer to the resolved prefix string. Never null.
	 */
	static const FString* ResolveBlueprintPrefixFromTag(const FAssetData& AssetData);
};
