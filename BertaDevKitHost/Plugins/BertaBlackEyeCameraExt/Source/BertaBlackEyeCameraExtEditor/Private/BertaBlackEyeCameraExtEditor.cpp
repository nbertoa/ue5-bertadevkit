#include "Modules/ModuleManager.h"

#include "Actors/BlackEyeCineCameraActorBase.h"
#include "Actors/Gameplay/BlackEyeOrbitCineCameraActor.h"
#include "BertaBlackEyeCameraRevealComponent.h"
#include "BertaBlackEyeCameraSwitcherComponent.h"
#include "BertaBlackEyeCameraTrigger.h"
#include "BertaBlackEyeRevealPreset.h"
#include "BertaBlackEyeRevealComponentVisualizer.h"
#include "Components/BoxComponent.h"
#include "Editor.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/Selection.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Commands/UIAction.h"
#include "ScopedTransaction.h"
#include "Textures/SlateIcon.h"
#include "ToolMenus.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "BertaBlackEyeCameraExtEditor"

DEFINE_LOG_CATEGORY_STATIC(LogBertaBlackEyeEditor, Log, All);

namespace
{
bool IsValidTiming(const FBertaBlackEyeRevealSettings& Settings)
{
    return FMath::IsFinite(Settings.BlendInTime) && Settings.BlendInTime >= 0.0f &&
        FMath::IsFinite(Settings.HoldTime) && Settings.HoldTime >= 0.0f &&
        FMath::IsFinite(Settings.BlendOutTime) && Settings.BlendOutTime >= 0.0f &&
        FMath::IsFinite(Settings.BlendInExponent) && Settings.BlendInExponent >= 0.0f &&
        FMath::IsFinite(Settings.BlendOutExponent) && Settings.BlendOutExponent >= 0.0f;
}

void ReportCameraCaveats(const ABlackEyeCineCameraActorBase* Camera, const AActor* Consumer)
{
    if (!IsValid(Camera)) return;
    UE_LOG(LogBertaBlackEyeEditor, Display, TEXT("%s uses Black Eye camera %s (%s)."),
        *GetNameSafe(Consumer), *GetNameSafe(Camera), *GetNameSafe(Camera->GetClass()));
    if (Camera->bSetControlRotation)
    {
        UE_LOG(LogBertaBlackEyeEditor, Display,
            TEXT("%s: Set Control Rotation is enabled; reveal restoration accounts for it."), *GetNameSafe(Camera));
    }
    if (Camera->GetAutoActivatePlayerIndex() != INDEX_NONE)
    {
        UE_LOG(LogBertaBlackEyeEditor, Display,
            TEXT("%s: AutoActivateForPlayer is enabled; confirm the temporary reveal and auto-activation fit your setup."),
            *GetNameSafe(Camera));
    }
    if (Camera->IsA<ABlackEyeOrbitCineCameraActor>())
    {
        UE_LOG(LogBertaBlackEyeEditor, Warning,
            TEXT("%s: Orbit binds Enhanced Input actions on BecomeViewTarget; verify residual bindings after return."),
            *GetNameSafe(Camera));
    }
}

void AuditReveal(const UBertaBlackEyeCameraRevealComponent* Reveal, bool bRequiredTarget)
{
    if (!IsValid(Reveal)) return;
    const AActor* Owner = Reveal->GetOwner();
    const bool bConfigured = bRequiredTarget || IsValid(Reveal->TargetCamera) || IsValid(Reveal->RevealPreset) ||
        Reveal->bNotifyParticipants || !Reveal->Participants.IsEmpty() ||
        Reveal->ReturnTargetPolicy == EBertaBlackEyeReturnTargetPolicy::ExplicitTarget;
    if (!bConfigured) return;

    UE_LOG(LogBertaBlackEyeEditor, Display, TEXT("%s: Return=%s ExternalChange=%s."),
        *GetNameSafe(Owner),
        *StaticEnum<EBertaBlackEyeReturnTargetPolicy>()->GetNameStringByValue(static_cast<int64>(Reveal->ReturnTargetPolicy)),
        *StaticEnum<EBertaBlackEyeExternalCameraChangePolicy>()->GetNameStringByValue(static_cast<int64>(Reveal->ExternalCameraChangePolicy)));
    if (Reveal->ReturnTargetPolicy == EBertaBlackEyeReturnTargetPolicy::ExplicitTarget &&
        !IsValid(Reveal->ExplicitReturnTarget))
    {
        UE_LOG(LogBertaBlackEyeEditor, Error,
            TEXT("%s: ExplicitTarget return policy requires a valid ExplicitReturnTarget."), *GetNameSafe(Owner));
    }

    if (!IsValid(Reveal->TargetCamera))
    {
        UE_LOG(LogBertaBlackEyeEditor, Error, TEXT("%s: configured reveal has no valid TargetCamera."), *GetNameSafe(Owner));
    }
    else
    {
        ReportCameraCaveats(Reveal->TargetCamera, Owner);
    }

    const FBertaBlackEyeRevealSettings& Settings = IsValid(Reveal->RevealPreset)
        ? Reveal->RevealPreset->Settings : Reveal->InlineSettings;
    if (!IsValidTiming(Settings))
    {
        UE_LOG(LogBertaBlackEyeEditor, Error,
            TEXT("%s: reveal timing or exponent is negative or non-finite (%s settings)."),
            *GetNameSafe(Owner), IsValid(Reveal->RevealPreset) ? TEXT("preset") : TEXT("inline"));
    }
}

void AuditSwitcher(const UBertaBlackEyeCameraSwitcherComponent* Switcher)
{
    if (!IsValid(Switcher)) return;
    TSet<const ABlackEyeCineCameraActorBase*> Seen;
    for (int32 Index = 0; Index < Switcher->Cameras.Num(); ++Index)
    {
        const ABlackEyeCineCameraActorBase* Camera = Switcher->Cameras[Index];
        if (!IsValid(Camera))
        {
            UE_LOG(LogBertaBlackEyeEditor, Error, TEXT("%s: switcher camera entry %d is null or destroyed."),
                *GetNameSafe(Switcher->GetOwner()), Index);
        }
        else if (Seen.Contains(Camera))
        {
            UE_LOG(LogBertaBlackEyeEditor, Error, TEXT("%s: duplicate switcher camera %s at entry %d."),
                *GetNameSafe(Switcher->GetOwner()), *GetNameSafe(Camera), Index);
        }
        else
        {
            Seen.Add(Camera);
        }
    }
    if (!FMath::IsFinite(Switcher->BlendTime) || Switcher->BlendTime < 0.0f ||
        !FMath::IsFinite(Switcher->BlendExponent) || Switcher->BlendExponent < 0.0f)
    {
        UE_LOG(LogBertaBlackEyeEditor, Error, TEXT("%s: switcher blend timing is negative or non-finite."),
            *GetNameSafe(Switcher->GetOwner()));
    }
}
}

class FBertaBlackEyeCameraExtEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        RegisterVisualizer();
        UToolMenus::RegisterStartupCallback(
            FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBertaBlackEyeCameraExtEditorModule::RegisterMenus));
    }

    virtual void ShutdownModule() override
    {
        if (bVisualizerRegistered && GUnrealEd)
        {
            GUnrealEd->UnregisterComponentVisualizer(UBertaBlackEyeCameraRevealComponent::StaticClass()->GetFName());
        }
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
    }

private:
    void RegisterVisualizer()
    {
        if (!bVisualizerRegistered && GUnrealEd)
        {
            GUnrealEd->RegisterComponentVisualizer(UBertaBlackEyeCameraRevealComponent::StaticClass()->GetFName(),
                MakeShared<FBertaBlackEyeRevealComponentVisualizer>());
            bVisualizerRegistered = true;
        }
    }

    void RegisterMenus()
    {
        RegisterVisualizer();
        UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
        if (!ToolsMenu) return;

        FToolMenuSection& Section = ToolsMenu->AddSection(TEXT("BertaBlackEyeCamera"),
            LOCTEXT("BertaBlackEyeSection", "Berta Black Eye Camera"));

        FToolMenuEntry CreateEntry = FToolMenuEntry::InitMenuEntry(TEXT("BertaBlackEyeCreateReveal"),
            LOCTEXT("CreateReveal", "Create Berta Reveal Trigger"),
            LOCTEXT("CreateRevealTip", "Create an undoable reveal trigger for the one selected Black Eye camera."),
            FSlateIcon(), FUIAction(FExecuteAction::CreateRaw(this, &FBertaBlackEyeCameraExtEditorModule::CreateTriggerForSelectedCamera)));
        CreateEntry.Owner = FToolMenuOwner(this);
        Section.AddEntry(CreateEntry);

        FToolMenuEntry AroundEntry = FToolMenuEntry::InitMenuEntry(TEXT("BertaBlackEyeCreateRevealAround"),
            LOCTEXT("CreateRevealAround", "Create Berta Reveal Trigger Around Actor"),
            LOCTEXT("CreateRevealAroundTip", "Select exactly one Black Eye camera and one gameplay actor."),
            FSlateIcon(), FUIAction(FExecuteAction::CreateRaw(this, &FBertaBlackEyeCameraExtEditorModule::CreateTriggerAroundSelectedActor)));
        AroundEntry.Owner = FToolMenuOwner(this);
        Section.AddEntry(AroundEntry);

        FToolMenuEntry AuditEntry = FToolMenuEntry::InitMenuEntry(TEXT("BertaBlackEyeAudit"),
            LOCTEXT("Audit", "Audit Black Eye Camera Setup"),
            LOCTEXT("AuditTip", "Report setup issues in Output Log without changing actors or assets."),
            FSlateIcon(), FUIAction(FExecuteAction::CreateRaw(this, &FBertaBlackEyeCameraExtEditorModule::RunAudit)));
        AuditEntry.Owner = FToolMenuOwner(this);
        Section.AddEntry(AuditEntry);
    }

    bool ResolveSelection(ABlackEyeCineCameraActorBase*& OutCamera, AActor*& OutOtherActor, bool bAroundActor) const
    {
        OutCamera = nullptr;
        OutOtherActor = nullptr;
        if (!GEditor || !GEditor->GetSelectedActors()) return false;

        int32 ActorCount = 0;
        for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
        {
            AActor* Selected = Cast<AActor>(*It);
            if (!IsValid(Selected)) continue;
            ++ActorCount;
            if (ABlackEyeCineCameraActorBase* Camera = Cast<ABlackEyeCineCameraActorBase>(Selected))
            {
                if (OutCamera) return false;
                OutCamera = Camera;
            }
            else
            {
                if (OutOtherActor) return false;
                OutOtherActor = Selected;
            }
        }
        return bAroundActor ? ActorCount == 2 && OutCamera && OutOtherActor : ActorCount == 1 && OutCamera;
    }

    void CreateTrigger(bool bAroundActor)
    {
        ABlackEyeCineCameraActorBase* Camera = nullptr;
        AActor* OtherActor = nullptr;
        if (!ResolveSelection(Camera, OtherActor, bAroundActor))
        {
            UE_LOG(LogBertaBlackEyeEditor, Warning, TEXT("Create Reveal Trigger requires %s."),
                bAroundActor ? TEXT("exactly one Black Eye camera and one other actor selected")
                             : TEXT("exactly one Black Eye camera selected"));
            return;
        }
        if (!IsValid(Camera->GetLevel()) || !IsValid(Camera->GetWorld())) return;

        FVector Location = Camera->GetActorLocation();
        FVector Extent(100.0f);
        if (bAroundActor)
        {
            OtherActor->GetActorBounds(false, Location, Extent);
            Extent += FVector(100.0f);
        }

        const FScopedTransaction Transaction(LOCTEXT("CreateRevealTransaction", "Create Berta Reveal Trigger"));
        Camera->GetLevel()->Modify();
        ABertaBlackEyeCameraTrigger* Trigger = Cast<ABertaBlackEyeCameraTrigger>(
            GEditor->AddActor(Camera->GetLevel(), ABertaBlackEyeCameraTrigger::StaticClass(), FTransform(Location)));
        if (!IsValid(Trigger))
        {
            UE_LOG(LogBertaBlackEyeEditor, Error, TEXT("Could not create Berta Reveal Trigger in %s."),
                *GetNameSafe(Camera->GetLevel()));
            return;
        }

        Trigger->Modify();
        Trigger->RevealComponent->Modify();
        Trigger->RevealComponent->TargetCamera = Camera;
        Trigger->RevealComponent->PostEditChange();
        if (bAroundActor)
        {
            Trigger->BoxCollision->Modify();
            Trigger->BoxCollision->SetBoxExtent(Extent);
            Trigger->BoxCollision->PostEditChange();
        }
        GEditor->SelectNone(false, true);
        GEditor->SelectActor(Trigger, true, true);
        UE_LOG(LogBertaBlackEyeEditor, Display, TEXT("Created %s for %s."),
            *GetNameSafe(Trigger), *GetNameSafe(Camera));
    }

    void CreateTriggerForSelectedCamera() { CreateTrigger(false); }
    void CreateTriggerAroundSelectedActor() { CreateTrigger(true); }

    void RunAudit()
    {
        UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
        if (!IsValid(World))
        {
            UE_LOG(LogBertaBlackEyeEditor, Warning, TEXT("Audit requires an open Level Editor world."));
            return;
        }

        int32 TriggerCount = 0;
        int32 RevealCount = 0;
        int32 SwitcherCount = 0;
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            const bool bTrigger = Actor->IsA<ABertaBlackEyeCameraTrigger>();
            if (bTrigger)
            {
                ++TriggerCount;
                const ABertaBlackEyeCameraTrigger* Trigger = CastChecked<ABertaBlackEyeCameraTrigger>(Actor);
                UE_LOG(LogBertaBlackEyeEditor, Display,
                    TEXT("Trigger %s: EndMode=%s TriggerOnce=%s Enabled=%s Config=%s."),
                    *GetNameSafe(Trigger),
                    *StaticEnum<EBertaBlackEyeRevealEndMode>()->GetNameStringByValue(static_cast<int64>(Trigger->EndMode)),
                    Trigger->bTriggerOnce ? TEXT("true") : TEXT("false"),
                    Trigger->bEnabled ? TEXT("true") : TEXT("false"),
                    IsValid(Trigger->RevealComponent->RevealPreset) ? TEXT("preset") : TEXT("inline"));
            }

            TArray<UBertaBlackEyeCameraRevealComponent*> Reveals;
            Actor->GetComponents(Reveals);
            for (const UBertaBlackEyeCameraRevealComponent* Reveal : Reveals)
            {
                ++RevealCount;
                AuditReveal(Reveal, bTrigger);
            }

            TArray<UBertaBlackEyeCameraSwitcherComponent*> Switchers;
            Actor->GetComponents(Switchers);
            for (const UBertaBlackEyeCameraSwitcherComponent* Switcher : Switchers)
            {
                ++SwitcherCount;
                AuditSwitcher(Switcher);
            }
        }
        UE_LOG(LogBertaBlackEyeEditor, Display,
            TEXT("Black Eye setup audit complete: triggers=%d reveal components=%d switchers=%d. No changes made."),
            TriggerCount, RevealCount, SwitcherCount);
    }

    bool bVisualizerRegistered = false;
};

IMPLEMENT_MODULE(FBertaBlackEyeCameraExtEditorModule, BertaBlackEyeCameraExtEditor)

#undef LOCTEXT_NAMESPACE
