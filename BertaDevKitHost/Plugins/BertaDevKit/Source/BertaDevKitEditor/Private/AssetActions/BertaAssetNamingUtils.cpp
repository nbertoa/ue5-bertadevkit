#include "AssetActions/BertaAssetNamingUtils.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EditorUtilityLibrary.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AimOffsetBlendSpace1D.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/UserDefinedEnum.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialParameterCollection.h"
#include "NiagaraEmitter.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "StructUtils/UserDefinedStruct.h"

// ----------------------------------------------------------------
// GetPrefixMap
// ----------------------------------------------------------------

const TMap<UClass*, FString>& UBertaAssetNamingUtils::GetPrefixMap()
{
	// Static local — constructed once on first call, reused for the lifetime
	// of the editor session. No locking needed: asset operations are single-threaded.
	//
	// Classes from optional plugins (GameplayAbilities, etc.) are intentionally
	// absent. Their prefixes are resolved by UBertaAssetAuditor::FindPrefixForClass
	// via class name string comparison, avoiding a hard module dependency that
	// would prevent the editor from launching on projects without those plugins.
	static const TMap<UClass*, FString> PrefixMap = {
		// Blueprints — specific subclasses must appear before UBlueprint so that
		// the hierarchy walk in FindPrefixForClass matches the most derived type first.
		{UAnimBlueprint::StaticClass(), TEXT("ABP_")}, {UUserWidget::StaticClass(), TEXT("WBP_")},
		{UBlueprintGeneratedClass::StaticClass(), TEXT("BPI_")}, {UEnvQueryContext::StaticClass(), TEXT("EQSC_")},
		{UBlueprint::StaticClass(), TEXT("BP_")},

		// Meshes
		{UStaticMesh::StaticClass(), TEXT("SM_")}, {USkeletalMesh::StaticClass(), TEXT("SKM_")},

		// Materials
		{UMaterial::StaticClass(), TEXT("M_")}, {UMaterialInstanceConstant::StaticClass(), TEXT("MI_")},
		{UMaterialParameterCollection::StaticClass(), TEXT("MPC_")},

		// Textures
		{UTexture2D::StaticClass(), TEXT("T_")}, {UTextureCube::StaticClass(), TEXT("T_")},
		{UTextureRenderTarget2D::StaticClass(), TEXT("RT_")},

		// Animation — AimOffset is a BlendSpace subclass; must appear before BlendSpace.
		{UAimOffsetBlendSpace::StaticClass(), TEXT("AO_")}, {UAimOffsetBlendSpace1D::StaticClass(), TEXT("AO_")},
		{UAnimSequence::StaticClass(), TEXT("AS_")}, {UAnimMontage::StaticClass(), TEXT("AM_")},
		{UBlendSpace::StaticClass(), TEXT("BS_")}, {UBlendSpace1D::StaticClass(), TEXT("BS_")},

		// VFX
		{UParticleSystem::StaticClass(), TEXT("PS_")}, {UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")},

		// Audio
		{USoundWave::StaticClass(), TEXT("SW_")}, {USoundCue::StaticClass(), TEXT("SC_")},

		// Data
		{UDataTable::StaticClass(), TEXT("DT_")}, {UDataAsset::StaticClass(), TEXT("DA_")},

		// Physics
		{UPhysicsAsset::StaticClass(), TEXT("PA_")},

		// User-defined types
		{UUserDefinedEnum::StaticClass(), TEXT("E_")}, {UUserDefinedStruct::StaticClass(), TEXT("F_")},

		// Framework Blueprints — native parent classes for common framework BPs.
		// Resolved via the UBlueprint::ParentClass walk in FindPrefixForClass.
		{AGameModeBase::StaticClass(), TEXT("GM_")}, {AGameMode::StaticClass(), TEXT("GM_")},
		{APlayerController::StaticClass(), TEXT("PC_")}, {ACharacter::StaticClass(), TEXT("CH_")},
		{APawn::StaticClass(), TEXT("P_")},

		// AI
		{UBlackboardData::StaticClass(), TEXT("BB_")}, {AAIController::StaticClass(), TEXT("AIC_")},
		{UBTDecorator::StaticClass(), TEXT("BTD_")}, {UBTService::StaticClass(), TEXT("BTS_")},
		{UBTTaskNode::StaticClass(), TEXT("BTT_")},

		// EQS
		{UEnvQuery::StaticClass(), TEXT("EQS_")},
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
