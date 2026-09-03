#pragma once

#include "AssetRegistry/AssetData.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaAssetNamingUtils.generated.h"

enum class EBertaAssetNamingStatus : uint8 { AlreadyCorrect, NeedsRename, UnknownClass };

struct FBertaAssetNamingPlan
{
	EBertaAssetNamingStatus Status = EBertaAssetNamingStatus::UnknownClass;
	FString ExpectedPrefix;
	FString TargetName;
};

UENUM()
enum class EBertaRenameResult : uint8 { Renamed, AlreadyCorrect, UnknownClass, Failed };

/** C++-only owner of asset naming planning and execution. */
UCLASS()
class BERTADEVKITEDITOR_API UBertaAssetNamingUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static const TMap<UClass*, FString>& GetPrefixMap();
	static const TMap<FTopLevelAssetPath, FString>& GetOptionalPluginPrefixes();
	/** Resolves prefix, target name, and rename state without loading the asset. */
	static FBertaAssetNamingPlan BuildRenamePlan(const FAssetData& AssetData);
	/** Executes a single loaded-asset rename through the shared Asset Naming execution path. */
	static EBertaRenameResult ExecuteRename(UObject* Asset, const FBertaAssetNamingPlan& Plan);
	/** Convenience entry point for a loaded asset; it uses the same FAssetData plan. */
	static EBertaRenameResult RenameAssetWithPrefix(UObject* Asset);
};
