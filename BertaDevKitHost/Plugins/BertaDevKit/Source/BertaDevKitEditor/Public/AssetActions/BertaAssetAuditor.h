#pragma once

#include "UObject/Object.h"

#include "BertaAssetAuditor.generated.h"

/**
 * Scans assets in the project and reports or fixes naming convention violations.
 *
 * Operates on the current Content Browser selection if any assets are selected;
 * otherwise scans all assets under /Game/ recursively via the Asset Registry
 * without loading them into memory.
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
	 * Collects the asset scope: returns selected assets from the Content Browser
	 * if any are selected, otherwise returns all assets under /Game/ via Asset Registry.
	 *
	 * @param OutAssets  Populated with the resolved asset scope.
	 */
	static void ResolveAssetScope(TArray<FAssetData>& OutAssets);

	/**
	 * Walks the class hierarchy of AssetClass to find the nearest prefix registered
	 * in UBertaAssetNamingUtils::GetPrefixMap(). Returns nullptr if none is found.
	 *
	 * Extracted to avoid duplicating the walk logic between RunAudit and RunAuditAndFix.
	 *
	 * @param AssetClass  The class to start the walk from. Must not be null.
	 * @return            Pointer to the registered prefix string, or nullptr if unknown.
	 */
	static const FString* FindPrefixForClass(UClass* AssetClass);
};
