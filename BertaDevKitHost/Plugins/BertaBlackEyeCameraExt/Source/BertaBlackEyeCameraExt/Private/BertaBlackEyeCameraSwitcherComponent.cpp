#include "BertaBlackEyeCameraSwitcherComponent.h"

#include "Actors/BlackEyeCineCameraActorBase.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaBlackEyeSwitcher, Log, All);

UBertaBlackEyeCameraSwitcherComponent::UBertaBlackEyeCameraSwitcherComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

APlayerController* UBertaBlackEyeCameraSwitcherComponent::ResolveLocalPlayerController() const
{
    if (APlayerController* OwnerController = Cast<APlayerController>(GetOwner()))
    {
        return IsValid(OwnerController) && OwnerController->IsLocalPlayerController() ? OwnerController : nullptr;
    }

    const UWorld* World = GetWorld();
    if (!IsValid(World)) return nullptr;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        if (IsValid(PlayerController) && PlayerController->IsLocalPlayerController()) return PlayerController;
    }
    return nullptr;
}

bool UBertaBlackEyeCameraSwitcherComponent::IsCameraSelectionValid() const
{
    const UWorld* World = GetWorld();
    if (!IsValid(World) || Cameras.IsEmpty() || !FMath::IsFinite(BlendTime) || BlendTime < 0.0f ||
        !FMath::IsFinite(BlendExponent) || BlendExponent < 0.0f) return false;

    TSet<const ABlackEyeCineCameraActorBase*> Seen;
    for (const ABlackEyeCineCameraActorBase* Camera : Cameras)
    {
        if (!IsValid(Camera) || Camera->GetWorld() != World || Seen.Contains(Camera)) return false;
        Seen.Add(Camera);
    }
    return true;
}

int32 UBertaBlackEyeCameraSwitcherComponent::FindCurrentCameraIndex(APlayerController* PlayerController) const
{
    if (!IsValid(PlayerController)) return INDEX_NONE;
    return Cameras.IndexOfByPredicate([Current = PlayerController->GetViewTarget()](const TObjectPtr<ABlackEyeCineCameraActorBase>& Camera)
    {
        return Camera == Current;
    });
}

bool UBertaBlackEyeCameraSwitcherComponent::SelectCamera(int32 Index)
{
    if (!IsCameraSelectionValid() || !Cameras.IsValidIndex(Index))
    {
        UE_LOG(LogBertaBlackEyeSwitcher, Warning, TEXT("Camera selection failed: empty, invalid, duplicate or out-of-range configuration."));
        return false;
    }
    APlayerController* PlayerController = ResolveLocalPlayerController();
    if (!IsValid(PlayerController) || PlayerController->GetWorld() != GetWorld() ||
        !IsValid(PlayerController->PlayerCameraManager))
    {
        UE_LOG(LogBertaBlackEyeSwitcher, Warning, TEXT("Camera selection failed: no usable local PlayerController."));
        return false;
    }

    if (PlayerController->GetViewTarget() != Cameras[Index])
    {
        PlayerController->SetViewTargetWithBlend(Cameras[Index], BlendTime, BlendFunction.GetValue(),
            BlendExponent, bLockOutgoing);
    }
    return true;
}

bool UBertaBlackEyeCameraSwitcherComponent::SelectNextCamera()
{
    if (!IsCameraSelectionValid()) return false;
    const int32 CurrentIndex = FindCurrentCameraIndex(ResolveLocalPlayerController());
    return SelectCamera(CurrentIndex == INDEX_NONE ? 0 : (CurrentIndex + 1) % Cameras.Num());
}

bool UBertaBlackEyeCameraSwitcherComponent::SelectPreviousCamera()
{
    if (!IsCameraSelectionValid()) return false;
    const int32 CurrentIndex = FindCurrentCameraIndex(ResolveLocalPlayerController());
    return SelectCamera(CurrentIndex == INDEX_NONE ? Cameras.Num() - 1 :
        (CurrentIndex + Cameras.Num() - 1) % Cameras.Num());
}

int32 UBertaBlackEyeCameraSwitcherComponent::GetSelectedCameraIndex() const
{
    if (!IsCameraSelectionValid()) return INDEX_NONE;
    return FindCurrentCameraIndex(ResolveLocalPlayerController());
}

ABlackEyeCineCameraActorBase* UBertaBlackEyeCameraSwitcherComponent::GetSelectedCamera() const
{
    const int32 Index = GetSelectedCameraIndex();
    return Cameras.IsValidIndex(Index) ? Cameras[Index] : nullptr;
}
