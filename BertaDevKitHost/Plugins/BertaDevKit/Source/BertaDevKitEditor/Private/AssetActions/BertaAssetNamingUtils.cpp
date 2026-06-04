#include "AssetActions/BertaAssetNamingUtils.h"

#include "Log/BertaDevKitEditorLog.h"

#include "EditorUtilityLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/BlendSpace.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AimOffsetBlendSpace1D.h"
#include "Animation/BlendSpace1D.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialParameterCollection.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "Particles/ParticleSystem.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "StructUtils/UserDefinedStruct.h"  // UE 5.6+ canonical include for UUserDefinedStruct
#include "Engine/UserDefinedEnum.h"

// ----------------------------------------------------------------
// GetPrefixMap
// ----------------------------------------------------------------

const TMap<UClass*, FString>& UBertaAssetNamingUtils::GetPrefixMap()
{
	// Static local — constructed once on first call, reused for the lifetime of the editor session.
	// No locking needed: the editor is single-threaded for asset operations.
	static const TMap<UClass*, FString> PrefixMap = {
		{UBlueprint::StaticClass(), TEXT("BP_")}, {UAnimBlueprint::StaticClass(), TEXT("ABP_")},
		{UUserWidget::StaticClass(), TEXT("WBP_")}, {UStaticMesh::StaticClass(), TEXT("SM_")},
		{USkeletalMesh::StaticClass(), TEXT("SKM_")}, {UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(), TEXT("MI_")},
		{UMaterialParameterCollection::StaticClass(), TEXT("MPC_")}, {UTexture2D::StaticClass(), TEXT("T_")},
		{UTextureCube::StaticClass(), TEXT("T_")}, {UTextureRenderTarget2D::StaticClass(), TEXT("RT_")},
		{UAnimSequence::StaticClass(), TEXT("AS_")}, {UAnimMontage::StaticClass(), TEXT("AM_")},
		{UBlendSpace::StaticClass(), TEXT("BS_")}, {UBlendSpace1D::StaticClass(), TEXT("BS_")},
		{UAimOffsetBlendSpace::StaticClass(), TEXT("AO_")}, {UAimOffsetBlendSpace1D::StaticClass(), TEXT("AO_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")}, {UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")}, {USoundWave::StaticClass(), TEXT("SW_")},
		{USoundCue::StaticClass(), TEXT("SC_")}, {UDataTable::StaticClass(), TEXT("DT_")},
		{UDataAsset::StaticClass(), TEXT("DA_")}, {UPhysicsAsset::StaticClass(), TEXT("PA_")},
		{UUserDefinedEnum::StaticClass(), TEXT("E_")}, {UUserDefinedStruct::StaticClass(), TEXT("F_")},
	};

	return PrefixMap;
}

// ----------------------------------------------------------------
// RenameAssetWithPrefix
// ----------------------------------------------------------------

EBertaRenameResult UBertaAssetNamingUtils::RenameAssetWithPrefix(UObject* const Asset)
{
	// Programming invariant — callers must validate before passing.
	if (!ensureMsgf(IsValid(Asset),
	                TEXT("UBertaAssetNamingUtils::RenameAssetWithPrefix — received null or invalid asset.")))
	{
		return EBertaRenameResult::UnknownClass;
	}

	const TMap<UClass*, FString>& PrefixMap = GetPrefixMap();

	// Walk up the class hierarchy to find the nearest registered prefix.
	// This correctly handles Blueprint subclasses of known types (e.g. a BP
	// subclassing UDataAsset will match UDataAsset's "DA_" entry).
	UClass* CurrentClass = Asset->GetClass();
	const FString* FoundPrefix = nullptr;

	while (CurrentClass)
	{
		FoundPrefix = PrefixMap.Find(CurrentClass);

		if (FoundPrefix)
		{
			break;
		}

		CurrentClass = CurrentClass->GetSuperClass();
	}

	if (!FoundPrefix || FoundPrefix->IsEmpty())
	{
		// Unknown class — not a violation, just not tracked. Caller decides how to log this.
		return EBertaRenameResult::UnknownClass;
	}

	// Asset already carries the correct prefix — nothing to do.
	if (Asset->GetName().StartsWith(*FoundPrefix))
	{
		return EBertaRenameResult::AlreadyCorrect;
	}

	// Take a mutable copy of the name to apply stripping and prefixing.
	FString CleanName = Asset->GetName();

	// Material instances auto-named by the engine may carry "M_" and "_Inst".
	// Strip both before applying "MI_" to produce a clean final name.
	if (Asset->IsA<UMaterialInstanceConstant>())
	{
		CleanName.RemoveFromStart(TEXT("M_"));
		CleanName.RemoveFromEnd(TEXT("_Inst"));
	}

	const FString NewName = *FoundPrefix + CleanName;
	UEditorUtilityLibrary::RenameAsset(Asset,
	                                   NewName);

	return EBertaRenameResult::Renamed;
}
