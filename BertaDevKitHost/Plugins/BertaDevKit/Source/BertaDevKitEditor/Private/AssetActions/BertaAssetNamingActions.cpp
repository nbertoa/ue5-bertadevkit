#include "AssetActions/BertaAssetNamingActions.h"

#include "AssetActions/BertaAssetNamingUtils.h"
#include "EditorUtilityLibrary.h"
#include "Log/BertaDevKitEditorLog.h"

// ----------------------------------------------------------------
// AddPrefixes
// ----------------------------------------------------------------

void UBertaAssetNamingActions::AddPrefixes()
{
	const TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	if (SelectedObjects.IsEmpty())
	{
		UE_LOG(LogBertaDevKitEditor,
		       Warning,
		       TEXT("[UBertaAssetNamingActions::AddPrefixes] No assets selected."));
		return;
	}

	int32 RenamedCount = 0;

	for (UObject* const SelectedObject : SelectedObjects)
	{
		if (!IsValid(SelectedObject))
		{
			UE_LOG(LogBertaDevKitEditor,
			       Warning,
			       TEXT("[UBertaAssetNamingActions::AddPrefixes] Skipping null or invalid asset."));
			continue;
		}

		const EBertaRenameResult Result = UBertaAssetNamingUtils::RenameAssetWithPrefix(SelectedObject);

		switch (Result)
		{
		case EBertaRenameResult::Renamed:
			{
				UE_LOG(LogBertaDevKitEditor,
				       Log,
				       TEXT("[UBertaAssetNamingActions::AddPrefixes] Renamed: %s"),
				       *SelectedObject->GetName());
				++RenamedCount;
				break;
			}

		case EBertaRenameResult::AlreadyCorrect:
			{
				UE_LOG(LogBertaDevKitEditor,
				       Log,
				       TEXT("[UBertaAssetNamingActions::AddPrefixes] Already correct: %s"),
				       *SelectedObject->GetName());
				break;
			}

		case EBertaRenameResult::UnknownClass:
			{
				UE_LOG(LogBertaDevKitEditor,
				       Warning,
				       TEXT("[UBertaAssetNamingActions::AddPrefixes] Unknown class, skipping: %s (%s)"),
				       *SelectedObject->GetName(),
				       *SelectedObject->GetClass()->GetName());
				break;
			}

		default:
			{
				// A new EBertaRenameResult value was added without updating this switch.
				// Fail loudly in development so the gap is caught immediately.
				ensureMsgf(false,
				           TEXT(
					           "[UBertaAssetNamingActions::AddPrefixes] Unhandled EBertaRenameResult value %d — add a case for it."
				           ),
				           static_cast<int32>(Result));
				break;
			}
		}
	}

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetNamingActions::AddPrefixes] Done — renamed %d asset(s)."),
	       RenamedCount);
}
