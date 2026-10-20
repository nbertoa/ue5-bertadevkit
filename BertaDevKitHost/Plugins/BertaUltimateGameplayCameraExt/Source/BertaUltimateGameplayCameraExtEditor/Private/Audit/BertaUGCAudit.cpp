#include "Audit/BertaUGCAudit.h"

#include "Camera/Data/UGC_CameraData.h"
#include "Camera/UGC_PlayerCameraManager.h"
#include "Components/BertaUGCCameraCycleComponent.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Camera/CameraComponent.h"
#include "Containers/Set.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameMapsSettings.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableSet.h"
#include "Modules/ModuleManager.h"
#include "Pawn/UGC_PawnInterface.h"
#include "UObject/Class.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaUGCAudit, Log, All);

namespace
{
	enum class ESeverity { Error, Warning, Info };

	void Report(ESeverity Severity, const FString& Path, const FString& Symbol, const FString& Message)
	{
		const FString Line = FString::Printf(TEXT("[UGC Audit] [%s] %s :: %s: %s"),
			Severity == ESeverity::Error ? TEXT("ERROR") : Severity == ESeverity::Warning ? TEXT("WARNING") : TEXT("INFO"),
			*Path, *Symbol, *Message);
		if (Severity == ESeverity::Error) { UE_LOG(LogBertaUGCAudit, Error, TEXT("%s"), *Line); }
		else if (Severity == ESeverity::Warning) { UE_LOG(LogBertaUGCAudit, Warning, TEXT("%s"), *Line); }
		else { UE_LOG(LogBertaUGCAudit, Display, TEXT("%s"), *Line); }
	}

	template<typename T>
	T* FindDeclaredComponent(const UClass* ActorClass)
	{
		if (!ActorClass) return nullptr;
		if (const AActor* CDO = Cast<AActor>(ActorClass->GetDefaultObject()))
		{
			if (T* Native = CDO->FindComponentByClass<T>()) return Native;
		}
		for (const UClass* Class = ActorClass; Class; Class = Class->GetSuperClass())
		{
			const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(Class);
			if (!BPClass || !BPClass->SimpleConstructionScript) continue;
			for (const USCS_Node* Node : BPClass->SimpleConstructionScript->GetAllNodes())
			{
				if (Node && Node->ComponentClass && Node->ComponentClass->IsChildOf(T::StaticClass()))
				{
					return Cast<T>(Node->ComponentTemplate);
				}
			}
		}
		return nullptr;
	}

	bool HasCompiledDefaults(const UClass* Class, FString& OutReason)
	{
		for (const UClass* Current = Class; Current; Current = Current->GetSuperClass())
		{
			const UBlueprint* Blueprint = Cast<UBlueprint>(Current->ClassGeneratedBy);
			if (Current->HasAnyClassFlags(CLASS_NewerVersionExists) || (Blueprint && !Blueprint->IsUpToDate()))
			{
				OutReason = FString::Printf(TEXT("Class %s has stale or uncompiled Blueprint defaults; compile it before auditing."), *Current->GetPathName());
				return false;
			}
		}
		return true;
	}

	void AuditCycle(const UClass* ControllerClass, bool bDefaultResolved, const UUGC_CameraDataAssetBase* DefaultData)
	{
		FString Reason;
		const UBertaUGCCameraCycleComponent* Cycle = BertaUGCAudit::ResolveCycleTemplate(ControllerClass, Reason);
		if (!Reason.IsEmpty())
		{
			Report(ESeverity::Warning, ControllerClass->GetPathName(), TEXT("CameraPresets"), TEXT("INCONCLUSIVE: ") + Reason);
			return;
		}
		if (!Cycle)
		{
			Report(ESeverity::Info, ControllerClass->GetPathName(), TEXT("CameraCycle"), TEXT("No Berta camera cycle component declared; optional. No CameraPresets array to compare."));
			return;
		}
		const FString Path = Cycle->GetPathName();
		Report(ESeverity::Info, Path, TEXT("CameraPresets"), FString::Printf(TEXT("Effective component/template; %d entries."), Cycle->CameraPresets.Num()));
		bool bValidCycle = !Cycle->CameraPresets.IsEmpty();
		if (Cycle->CameraPresets.IsEmpty())
		{
			Report(ESeverity::Warning, Path, TEXT("CameraPresets"), TEXT("Camera cycle has no presets and cannot select a camera."));
		}
		TSet<const UUGC_CameraDataAssetBase*> Seen;
		for (int32 Index = 0; Index < Cycle->CameraPresets.Num(); ++Index)
		{
			const UUGC_CameraDataAssetBase* Preset = Cycle->CameraPresets[Index].Get();
			const FString Symbol = FString::Printf(TEXT("CameraPresets[%d]"), Index);
			const FString AssetPath = GetPathNameSafe(Preset);
			if (!IsValid(Preset))
			{
				Report(ESeverity::Warning, Path, Symbol, AssetPath + TEXT("; null or invalid preset invalidates the cycle and blocks selection."));
				bValidCycle = false;
			}
			else if (Seen.Contains(Preset))
			{
				Report(ESeverity::Warning, Path, Symbol, AssetPath + TEXT("; duplicate preset invalidates the cycle because indices are ambiguous."));
				bValidCycle = false;
			}
			else
			{
				Report(ESeverity::Info, Path, Symbol, AssetPath);
				Seen.Add(Preset);
			}
		}
		if (!bValidCycle || !bDefaultResolved || !DefaultData) return;

		const int32 DefaultIndex = Cycle->CameraPresets.IndexOfByPredicate(
			[DefaultData](const TObjectPtr<UUGC_CameraDataAssetBase>& Preset) { return Preset.Get() == DefaultData; });
		if (DefaultIndex != INDEX_NONE)
		{
			Report(ESeverity::Info, Path, TEXT("DefaultCameraData"), FString::Printf(
				TEXT("Manager default %s matches CameraPresets[%d] by object identity; statically consistent membership, not a runtime success guarantee."),
				*DefaultData->GetPathName(), DefaultIndex));
		}
		else
		{
			Report(ESeverity::Warning, Path, TEXT("DefaultCameraData"), FString::Printf(
				TEXT("Manager default %s is absent from the CameraPresets listed above. If it becomes active, Next/Previous and Select reject changes while that asset remains outside the cycle. In the manager's Class Defaults choose a default from the cycle (prefer a project-owned asset), or include this default in the component's CameraPresets only if it should be selectable. Any array index is valid. This conditional risk does not prove the cause of a previous success=false."),
				*DefaultData->GetPathName()));
		}
	}

	void AuditProjectSetup()
	{
		const FString GameModePath = UGameMapsSettings::GetGlobalDefaultGameMode();
		UClass* GameModeClass = FSoftClassPath(GameModePath).TryLoadClass<AGameModeBase>();
		if (!GameModeClass)
		{
			Report(ESeverity::Error, TEXT("Project Settings/Maps & Modes"), TEXT("GlobalDefaultGameMode"), TEXT("Default GameMode class cannot be loaded."));
			return;
		}
		Report(ESeverity::Info, GameModeClass->GetPathName(), TEXT("GlobalDefaultGameMode"), TEXT("Inspecting global compiled Class Defaults; the active map may use another GameMode."));
		FString Reason;
		if (!HasCompiledDefaults(GameModeClass, Reason))
		{
			Report(ESeverity::Warning, GameModeClass->GetPathName(), TEXT("GlobalDefaultGameMode"), TEXT("INCONCLUSIVE: ") + Reason);
			return;
		}
		const AGameModeBase* GameMode = GameModeClass->GetDefaultObject<AGameModeBase>();
		UClass* ControllerClass = GameMode->PlayerControllerClass.Get();
		if (!ControllerClass)
		{
			Report(ESeverity::Error, GameModeClass->GetPathName(), TEXT("PlayerControllerClass"), TEXT("PlayerController class is not configured."));
			return;
		}
		Report(ESeverity::Info, ControllerClass->GetPathName(), TEXT("PlayerControllerClass"), TEXT("Configured by the global GameMode; dynamic replacements are not inspected."));
		if (!HasCompiledDefaults(ControllerClass, Reason))
		{
			Report(ESeverity::Warning, ControllerClass->GetPathName(), TEXT("PlayerControllerClass"), TEXT("INCONCLUSIVE: ") + Reason);
			return;
		}
		const APlayerController* Controller = ControllerClass->GetDefaultObject<APlayerController>();
		const UClass* ManagerClass = Controller->PlayerCameraManagerClass.Get();
		Report(ESeverity::Info, ControllerClass->GetPathName(), TEXT("PlayerCameraManagerClass"), GetPathNameSafe(ManagerClass));
		const bool bGlobalUsesUGCManager = ManagerClass && ManagerClass->IsChildOf(AUGC_PlayerCameraManager::StaticClass());
		bool bDefaultResolved = false;
		const UUGC_CameraDataAssetBase* DefaultData = nullptr;
		if (!bGlobalUsesUGCManager)
		{
			Report(ESeverity::Warning, ControllerClass->GetPathName(), TEXT("PlayerCameraManagerClass"),
				TEXT("Global Default GameMode does not configure AUGC_PlayerCameraManager; map-specific GameMode overrides are not inspected."));
		}
		else
		{
			bDefaultResolved = BertaUGCAudit::ResolveDefaultCameraData(ManagerClass, DefaultData, Reason);
			if (!bDefaultResolved)
				Report(ESeverity::Warning, ManagerClass->GetPathName(), TEXT("DefaultCameraData"), TEXT("INCONCLUSIVE: ") + Reason + TEXT(" No default/cycle compatibility conclusion."));
			else if (!DefaultData)
				Report(ESeverity::Info, ManagerClass->GetPathName(), TEXT("DefaultCameraData"), TEXT("Unassigned in effective Class Defaults; no static initial preset. Not an error: with no active runtime asset, Next selects index 0 and Previous the last. Runtime initialization may choose other data."));
			else
				Report(ESeverity::Info, ManagerClass->GetPathName(), TEXT("DefaultCameraData"), DefaultData->GetPathName() + TEXT(" (configured default, not the current runtime asset)."));
		}
		AuditCycle(ControllerClass, bDefaultResolved, DefaultData);
		if (!bGlobalUsesUGCManager) return;

		UClass* PawnClass = GameMode->DefaultPawnClass.Get();
		if (!PawnClass)
		{
			Report(ESeverity::Warning, GameModeClass->GetPathName(), TEXT("DefaultPawnClass"), TEXT("No default Pawn; runtime spawning or map overrides may supply one."));
			return;
		}
		USpringArmComponent* Arm = FindDeclaredComponent<USpringArmComponent>(PawnClass);
		if (!Arm)
		{
			Report(ESeverity::Error, PawnClass->GetPathName(), TEXT("SpringArmComponent"), TEXT("No declared SpringArm; UGC possession preparation requires one."));
		}
		else
		{
			Report(ESeverity::Info, PawnClass->GetPathName(), Arm->GetName(), TEXT("SpringArm declared."));
			if (!Arm->bUsePawnControlRotation)
				Report(ESeverity::Info, PawnClass->GetPathName(), Arm->GetName(), TEXT("UsePawnControlRotation is disabled; verify this is intentional for the UGC setup."));
		}
		if (!FindDeclaredComponent<UCameraComponent>(PawnClass))
			Report(ESeverity::Warning, PawnClass->GetPathName(), TEXT("CameraComponent"), TEXT("No declared CameraComponent; verify the gameplay camera/view target."));
		if (!PawnClass->ImplementsInterface(UUGC_PawnInterface::StaticClass()))
			Report(ESeverity::Info, PawnClass->GetPathName(), TEXT("UGC_PawnInterface"), TEXT("Pawn does not declare the optional UGC input interface; custom input hooks may be used."));
	}

	void AuditPreset(const UUGC_CameraDataAssetBase& Asset)
	{
		const FString Path = Asset.GetPathName();
		const auto Range = [&Path](float Min, float Max, const TCHAR* Symbol)
		{
			if (!FMath::IsFinite(Min) || !FMath::IsFinite(Max))
				Report(ESeverity::Error, Path, Symbol, TEXT("Range contains a non-finite value; UGC maps camera settings through these values."));
			else if (Min > Max)
				Report(ESeverity::Warning, Path, Symbol, TEXT("Min exceeds Max and reverses the mapped range; verify this is intentional."));
		};
		const auto Duration = [&Path](float Time, const TCHAR* Symbol)
		{
			if (!FMath::IsFinite(Time) || Time < 0.f)
				Report(ESeverity::Error, Path, Symbol, TEXT("Blend duration must be finite and non-negative."));
		};
		Range(Asset.ArmLengthSettings.MinArmLength, Asset.ArmLengthSettings.MaxArmLength, TEXT("ArmLengthSettings"));
		Range(Asset.FOVSettings.MinFOV, Asset.FOVSettings.MaxFOV, TEXT("FOVSettings"));
		Duration(Asset.ArmLengthSettings.ArmRangeBlendTime, TEXT("ArmLengthSettings.ArmRangeBlendTime"));
		Duration(Asset.FOVSettings.FOVRangeBlendTime, TEXT("FOVSettings.FOVRangeBlendTime"));
		Duration(Asset.ArmOffsetSettings.ArmSocketOffsetBlendTime, TEXT("ArmOffsetSettings.ArmSocketOffsetBlendTime"));
		Duration(Asset.ArmOffsetSettings.ArmTargetOffsetBlendTime, TEXT("ArmOffsetSettings.ArmTargetOffsetBlendTime"));
		if (Asset.PitchConstraints.bConstrainPitch)
			Range(Asset.PitchConstraints.LocalMinPitch, Asset.PitchConstraints.LocalMaxPitch, TEXT("PitchConstraints"));
		Report(ESeverity::Info, Path, TEXT("CameraDataAsset"), TEXT("Preset inspected with source-backed range and duration rules."));
	}

	void AuditExplicitBlueprintWrites(const UBlueprint& Blueprint)
	{
		TArray<UEdGraph*> Graphs;
		Blueprint.GetAllGraphs(Graphs);
		for (const UEdGraph* Graph : Graphs)
		{
			if (!Graph) continue;
			for (const UEdGraphNode* Node : Graph->Nodes)
			{
				if (!Node) continue;
				bool bCandidate = false;
				if (const UK2Node_VariableSet* Set = Cast<UK2Node_VariableSet>(Node))
				{
					const FProperty* Property = Set->VariableReference.ResolveMember<FProperty>(Blueprint.SkeletonGeneratedClass);
					const UClass* Owner = Property ? Property->GetOwner<UClass>() : nullptr;
					const FName Name = Set->VariableReference.GetMemberName();
					bCandidate = Owner && ((Owner->IsChildOf(USpringArmComponent::StaticClass()) &&
						(Name == TEXT("TargetArmLength") || Name == TEXT("SocketOffset") || Name == TEXT("TargetOffset"))) ||
						(Owner->IsChildOf(UCameraComponent::StaticClass()) && Name == TEXT("FieldOfView")));
				}
				else if (const UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
				{
					const UFunction* Function = Call->FunctionReference.ResolveMember<UFunction>(Blueprint.SkeletonGeneratedClass);
					const UClass* Owner = Function ? Function->GetOuterUClass() : nullptr;
					bCandidate = Owner && ((Owner->IsChildOf(UCameraComponent::StaticClass()) && Function->GetFName() == TEXT("SetFieldOfView")) ||
						(Owner->IsChildOf(AController::StaticClass()) && Function->GetFName() == TEXT("SetControlRotation")));
				}
				if (bCandidate)
				{
					const FString Symbol = FString::Printf(TEXT("%s/%s"), *Graph->GetName(), *Node->GetNodeTitle(ENodeTitleType::ListView).ToString());
					Report(ESeverity::Info, Blueprint.GetPathName(), Symbol, TEXT("Explicit camera-property write found; possible UGC conflict if this node runs on the active camera. Static graph inspection cannot prove runtime overlap."));
				}
			}
		}
	}
}

bool BertaUGCAudit::ResolveDefaultCameraData(const UClass* ManagerClass, const UUGC_CameraDataAssetBase*& OutData, FString& OutReason)
{
	OutData = nullptr;
	OutReason.Reset();
	if (!ManagerClass || !ManagerClass->IsChildOf(AUGC_PlayerCameraManager::StaticClass()))
	{
		OutReason = TEXT("No UGC manager class to inspect.");
		return false;
	}
	if (!HasCompiledDefaults(ManagerClass, OutReason)) return false;

	// Verified on the installed vendor Blueprint: DefaultCameraData is a hard object
	// property, displayed as Default Camera Data. FindFProperty includes ancestors.
	const FProperty* Property = FindFProperty<FProperty>(ManagerClass, TEXT("DefaultCameraData"));
	const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property);
	if (!ObjectProperty || ObjectProperty->ArrayDim != 1 || !ObjectProperty->PropertyClass ||
		!ObjectProperty->PropertyClass->IsChildOf(UUGC_CameraDataAssetBase::StaticClass()))
	{
		OutReason = TEXT("Known DefaultCameraData hard object property is missing or has an unsupported type (including soft references). No default was inferred from runtime stack state or display-name matching.");
		return false;
	}
	const UObject* CDO = ManagerClass->GetDefaultObject();
	if (!IsValid(CDO))
	{
		OutReason = TEXT("Effective manager Class Default Object is unavailable.");
		return false;
	}
	const UObject* Value = ObjectProperty->GetObjectPropertyValue_InContainer(CDO);
	OutData = Cast<UUGC_CameraDataAssetBase>(Value);
	if (Value && (!IsValid(OutData)))
	{
		OutData = nullptr;
		OutReason = TEXT("Configured DefaultCameraData references an invalid or incompatible object.");
		return false;
	}
	return true;
}

const UBertaUGCCameraCycleComponent* BertaUGCAudit::ResolveCycleTemplate(const UClass* ControllerClass, FString& OutReason)
{
	OutReason.Reset();
	if (!ControllerClass || !ControllerClass->IsChildOf(APlayerController::StaticClass()))
	{
		OutReason = TEXT("No PlayerController class to inspect.");
		return nullptr;
	}
	if (!HasCompiledDefaults(ControllerClass, OutReason)) return nullptr;
	const APlayerController* CDO = ControllerClass->GetDefaultObject<APlayerController>();
	TArray<UBertaUGCCameraCycleComponent*> Candidates;
	CDO->GetComponents(Candidates);
	UBlueprintGeneratedClass* ActualBPGC = Cast<UBlueprintGeneratedClass>(CDO->GetClass());
	for (const UClass* Class = ControllerClass; Class; Class = Class->GetSuperClass())
	{
		const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(Class);
		if (!BPClass || !BPClass->SimpleConstructionScript) continue;
		for (const USCS_Node* Node : BPClass->SimpleConstructionScript->GetAllNodes())
		{
			if (!IsValid(Node) || !Node->ComponentClass)
			{
				OutReason = TEXT("An SCS node or its component class is unavailable; effective cycle configuration cannot be established.");
				return nullptr;
			}
			if (!Node->ComponentClass->IsChildOf(UBertaUGCCameraCycleComponent::StaticClass())) continue;
			// UE consults existing UInheritableComponentHandler overrides from the most
			// derived class toward the declaring SCS. It does not create override records.
			UBertaUGCCameraCycleComponent* Template = ActualBPGC
				? Cast<UBertaUGCCameraCycleComponent>(Node->GetActualComponentTemplate(ActualBPGC)) : nullptr;
			if (!IsValid(Template))
			{
				OutReason = FString::Printf(TEXT("Effective cycle template for %s cannot be resolved; no parent-template fallback was used."), *Node->GetPathName());
				return nullptr;
			}
			Candidates.AddUnique(Template);
		}
	}
	if (Candidates.Num() > 1)
	{
		OutReason = TEXT("Multiple cycle components/templates; runtime component lookup order is not inferred:");
		for (const UBertaUGCCameraCycleComponent* Candidate : Candidates)
			OutReason += TEXT(" ") + GetPathNameSafe(Candidate);
		return nullptr;
	}
	if (Candidates.Num() == 1 && !IsValid(Candidates[0]))
	{
		OutReason = TEXT("Native cycle component is invalid.");
		return nullptr;
	}
	return Candidates.IsEmpty() ? nullptr : Candidates[0];
}

void BertaUGCAudit::Run()
{
	UE_LOG(LogBertaUGCAudit, Display, TEXT("[UGC Audit] Starting read-only project audit. Scope: global GameMode compiled defaults, /Game UGC presets and standalone Blueprint graphs. Map GameModes, dynamic controllers/managers and temporary active camera data may differ; static consistency does not guarantee Next succeeds."));
	AuditProjectSetup();
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	if (Registry.IsLoadingAssets())
	{
		Report(ESeverity::Warning, TEXT("/Game"), TEXT("AssetRegistry"), TEXT("Asset Registry is still gathering; preset and Blueprint graph checks are incomplete."));
		return;
	}
	FARFilter PresetFilter;
	PresetFilter.PackagePaths.Add(TEXT("/Game"));
	PresetFilter.bRecursivePaths = true;
	PresetFilter.bRecursiveClasses = true;
	PresetFilter.ClassPaths.Add(UUGC_CameraDataAssetBase::StaticClass()->GetClassPathName());
	TArray<FAssetData> Assets;
	Registry.GetAssets(PresetFilter, Assets);
	int32 InspectedPresets = 0;
	for (const FAssetData& Data : Assets)
	{
		if (const UUGC_CameraDataAssetBase* Preset = Cast<UUGC_CameraDataAssetBase>(Data.GetAsset()))
		{
			AuditPreset(*Preset);
			++InspectedPresets;
		}
		else Report(ESeverity::Warning, Data.GetSoftObjectPath().ToString(), TEXT("Load"), TEXT("Preset could not be loaded; audit incomplete."));
	}
	FARFilter BlueprintFilter;
	BlueprintFilter.PackagePaths.Add(TEXT("/Game"));
	BlueprintFilter.bRecursivePaths = true;
	BlueprintFilter.bRecursiveClasses = true;
	BlueprintFilter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Assets.Reset();
	Registry.GetAssets(BlueprintFilter, Assets);
	int32 InspectedBlueprints = 0;
	for (const FAssetData& Data : Assets)
	{
		if (const UBlueprint* Blueprint = Cast<UBlueprint>(Data.GetAsset()))
		{
			AuditExplicitBlueprintWrites(*Blueprint);
			++InspectedBlueprints;
		}
		else Report(ESeverity::Warning, Data.GetSoftObjectPath().ToString(), TEXT("Load"), TEXT("Blueprint could not be loaded; writer scan incomplete."));
	}
	UE_LOG(LogBertaUGCAudit, Display,
		TEXT("[UGC Audit] Static pass finished: %d UGC presets and %d standalone Blueprints inspected; review warnings and inconclusive findings above. Map level scripts, C++ writes, runtime conditions and map-specific GameMode overrides are outside this static pass."),
		InspectedPresets, InspectedBlueprints);
}
