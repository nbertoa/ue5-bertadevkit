#include "AssetActions/BertaAssetNamingUtils.h"
#include "Log/BertaDevKitEditorLog.h"

#include "AIController.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AimOffsetBlendSpace1D.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "AssetToolsModule.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Blueprint/BlueprintSupport.h"
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
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialParameterCollection.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Misc/PackageName.h"
#include "NiagaraEmitter.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "StructUtils/UserDefinedStruct.h"

namespace
{
	const FString GenericBlueprintPrefix(TEXT("BP_"));

	const FString* FindPrefixInHierarchy(UClass* Class)
	{
		const TMap<UClass*, FString>& Prefixes = UBertaAssetNamingUtils::GetPrefixMap();
		const TMap<FTopLevelAssetPath, FString>& OptionalPrefixes = UBertaAssetNamingUtils::GetOptionalPluginPrefixes();
		for (UClass* Current = Class; Current; Current = Current->GetSuperClass())
		{
			if (const FString* Prefix = Prefixes.Find(Current)) return Prefix;
			if (const FString* Prefix = OptionalPrefixes.Find(Current->GetClassPathName())) return Prefix;
		}
		return nullptr;
	}

	const FString* FindOptionalPrefixByPath(const FString& ExportPath)
	{
		for (const TPair<FTopLevelAssetPath, FString>& Pair : UBertaAssetNamingUtils::GetOptionalPluginPrefixes())
		{
			if (ExportPath.Contains(Pair.Key.ToString())) return &Pair.Value;
		}
		return nullptr;
	}

	const FString* ResolveBlueprintPrefix(const FAssetData& AssetData, UClass* AssetClass)
	{
		// Specific Blueprint asset types win; generic UBlueprint is resolved from its native parent.
		if (AssetClass != UBlueprint::StaticClass()) if (const FString* Prefix = FindPrefixInHierarchy(AssetClass)) return Prefix;
		FString NativeParentPath;
		if (AssetData.GetTagValue(FBlueprintTags::NativeParentClassPath, NativeParentPath) && !NativeParentPath.IsEmpty() && NativeParentPath != TEXT("None"))
		{
			if (const FString* Prefix = FindOptionalPrefixByPath(NativeParentPath)) return Prefix;
			NativeParentPath.RemoveFromStart(TEXT("Class'"));
			NativeParentPath.RemoveFromEnd(TEXT("'"));
			if (UClass* NativeParentClass = FindObject<UClass>(nullptr, *NativeParentPath)) if (const FString* Prefix = FindPrefixInHierarchy(NativeParentClass)) return Prefix;
		}
		return &GenericBlueprintPrefix;
	}

	FString BuildTargetName(const FString& AssetName, const FString& Prefix, UClass* AssetClass)
	{
		FString NameToPrefix = AssetName;
		if (AssetClass->IsChildOf(UMaterialInstanceConstant::StaticClass())) { NameToPrefix.RemoveFromStart(TEXT("M_")); NameToPrefix.RemoveFromEnd(TEXT("_Inst")); }
		else if (AssetClass->IsChildOf(UAnimMontage::StaticClass())) NameToPrefix.RemoveFromEnd(TEXT("_Montage"));
		return Prefix + NameToPrefix;
	}
}

const TMap<UClass*, FString>& UBertaAssetNamingUtils::GetPrefixMap()
{
	static const TMap<UClass*, FString> Prefixes = {
		{ UAnimBlueprint::StaticClass(), TEXT("ABP_") }, { UUserWidget::StaticClass(), TEXT("WBP_") }, { UBlueprint::StaticClass(), TEXT("BP_") },
		{ UStaticMesh::StaticClass(), TEXT("SM_") }, { USkeletalMesh::StaticClass(), TEXT("SKM_") },
		{ UMaterial::StaticClass(), TEXT("M_") }, { UMaterialInstanceConstant::StaticClass(), TEXT("MI_") }, { UMaterialParameterCollection::StaticClass(), TEXT("MPC_") },
		{ UTexture2D::StaticClass(), TEXT("T_") }, { UTextureCube::StaticClass(), TEXT("T_") }, { UTextureRenderTarget2D::StaticClass(), TEXT("RT_") },
		{ UAimOffsetBlendSpace::StaticClass(), TEXT("AO_") }, { UAimOffsetBlendSpace1D::StaticClass(), TEXT("AO_") }, { UAnimMontage::StaticClass(), TEXT("AM_") }, { UAnimSequence::StaticClass(), TEXT("AS_") }, { UBlendSpace::StaticClass(), TEXT("BS_") }, { UBlendSpace1D::StaticClass(), TEXT("BS_") },
		{ UParticleSystem::StaticClass(), TEXT("PS_") }, { UNiagaraSystem::StaticClass(), TEXT("NS_") }, { UNiagaraEmitter::StaticClass(), TEXT("NE_") }, { USoundWave::StaticClass(), TEXT("SW_") }, { USoundCue::StaticClass(), TEXT("SC_") },
		{ UDataTable::StaticClass(), TEXT("DT_") }, { UDataAsset::StaticClass(), TEXT("DA_") }, { UPhysicsAsset::StaticClass(), TEXT("PA_") }, { UUserDefinedEnum::StaticClass(), TEXT("E_") }, { UUserDefinedStruct::StaticClass(), TEXT("F_") },
		{ AGameModeBase::StaticClass(), TEXT("GM_") }, { AGameMode::StaticClass(), TEXT("GM_") }, { APlayerController::StaticClass(), TEXT("PC_") }, { ACharacter::StaticClass(), TEXT("CH_") }, { APawn::StaticClass(), TEXT("P_") },
		{ UBlackboardData::StaticClass(), TEXT("BB_") }, { AAIController::StaticClass(), TEXT("AIC_") }, { UBTDecorator::StaticClass(), TEXT("BTD_") }, { UBTService::StaticClass(), TEXT("BTS_") }, { UBTTaskNode::StaticClass(), TEXT("BTT_") },
		{ UEnvQuery::StaticClass(), TEXT("EQS_") }, { UEnvQueryContext::StaticClass(), TEXT("EQSC_") }, { UInputAction::StaticClass(), TEXT("IA_") }, { UInputMappingContext::StaticClass(), TEXT("IMC_") },
	};
	return Prefixes;
}

const TMap<FTopLevelAssetPath, FString>& UBertaAssetNamingUtils::GetOptionalPluginPrefixes()
{
	static const TMap<FTopLevelAssetPath, FString> Prefixes = {
		{ FTopLevelAssetPath(TEXT("/Script/GameplayAbilities"), TEXT("GameplayAbility")), TEXT("GA_") },
		{ FTopLevelAssetPath(TEXT("/Script/GameplayAbilities"), TEXT("GameplayEffect")), TEXT("GE_") },
		{ FTopLevelAssetPath(TEXT("/Script/GameplayAbilities"), TEXT("GameplayCueNotify_Static")), TEXT("GC_") },
		{ FTopLevelAssetPath(TEXT("/Script/GameplayAbilities"), TEXT("GameplayCueNotify_Actor")), TEXT("GC_") },
	};
	return Prefixes;
}

FBertaAssetNamingPlan UBertaAssetNamingUtils::BuildRenamePlan(const FAssetData& AssetData)
{
	FBertaAssetNamingPlan Plan;
	UClass* AssetClass = AssetData.GetClass();
	if (!AssetClass) return Plan;
	const FString* Prefix = AssetClass->IsChildOf(UBlueprint::StaticClass()) ? ResolveBlueprintPrefix(AssetData, AssetClass) : FindPrefixInHierarchy(AssetClass);
	if (!Prefix || Prefix->IsEmpty()) return Plan;
	Plan.ExpectedPrefix = *Prefix;
	Plan.TargetName = BuildTargetName(AssetData.AssetName.ToString(), Plan.ExpectedPrefix, AssetClass);
	Plan.Status = AssetData.AssetName.ToString() == Plan.TargetName ? EBertaAssetNamingStatus::AlreadyCorrect : EBertaAssetNamingStatus::NeedsRename;
	return Plan;
}

EBertaRenameResult UBertaAssetNamingUtils::ExecuteRename(UObject* Asset, const FBertaAssetNamingPlan& Plan)
{
	if (Plan.Status == EBertaAssetNamingStatus::UnknownClass) return EBertaRenameResult::UnknownClass;
	if (Plan.Status == EBertaAssetNamingStatus::AlreadyCorrect) return EBertaRenameResult::AlreadyCorrect;
	if (!ensureMsgf(IsValid(Asset), TEXT("ExecuteRename requires a valid asset for a NeedsRename plan."))) return EBertaRenameResult::Failed;
	TArray<FAssetRenameData> RenameData;
	RenameData.Emplace(Asset, FPackageName::GetLongPackagePath(Asset->GetOutermost()->GetName()), Plan.TargetName);
	if (!FAssetToolsModule::GetModule().Get().RenameAssets(RenameData))
	{
		UE_LOG(LogBertaDevKitEditor, Error, TEXT("Asset rename failed: %s -> %s"), *Asset->GetName(), *Plan.TargetName);
		return EBertaRenameResult::Failed;
	}
	return EBertaRenameResult::Renamed;
}

EBertaRenameResult UBertaAssetNamingUtils::RenameAssetWithPrefix(UObject* Asset)
{
	if (!ensureMsgf(IsValid(Asset), TEXT("RenameAssetWithPrefix received an invalid asset."))) return EBertaRenameResult::UnknownClass;
	return ExecuteRename(Asset, BuildRenamePlan(FAssetData(Asset)));
}
