#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaAssetNamingUtils.generated.h"

/**
 * Result of a single asset rename attempt performed by UBertaAssetNamingUtils::RenameAssetWithPrefix().
 * Used by callers to log and report outcomes without magic booleans.
 *
 * @note This enum is C++-only by design. If Blueprint exposure is needed in the future,
 *       add BlueprintType to the UENUM specifier and expose RenameAssetWithPrefix via UFUNCTION.
 */
UENUM()
enum class EBertaRenameResult : uint8
{
	/** Asset was successfully renamed with the correct prefix. */
	Renamed,

	/** Asset already carried the correct prefix — no action taken. */
	AlreadyCorrect,

	/** No prefix entry exists in the map for this asset's class — asset was skipped. */
	UnknownClass
};

/**
 * Sole owner of the asset prefix map and all prefix-related logic.
 * Consumed by UBertaAssetNamingActions and UBertaAssetAuditor.
 *
 * @note All functions are static — this class is never instantiated.
 */
UCLASS()
class BERTADEVKITEDITOR_API UBertaAssetNamingUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns the canonical map of UClass to expected name prefix.
	 * Stored as a static local — built once, reused across all callers.
	 *
	 * Classes from optional plugins (e.g. GameplayAbilities) are intentionally
	 * excluded to avoid hard module dependencies. Their prefixes are resolved
	 * by UBertaAssetAuditor::FindPrefixForClass via class name string comparison.
	 *
	 * @return Const reference to the prefix map. Never invalid.
	 */
	static const TMap<UClass*, FString>& GetPrefixMap();

	/**
	 * Applies the correct prefix to a single asset, based on GetPrefixMap().
	 * Handles UMaterialInstanceConstant stripping internally (removes "M_" and "_Inst"
	 * before applying "MI_").
	 *
	 * @param Asset     The asset to rename. Must be a valid non-null UObject.
	 * @return          The outcome of the rename attempt as EBertaRenameResult.
	 *
	 * @note Callers are responsible for logging the result — this function logs
	 *       only internal errors that indicate programming mistakes.
	 */
	static EBertaRenameResult RenameAssetWithPrefix(UObject* Asset);
};
