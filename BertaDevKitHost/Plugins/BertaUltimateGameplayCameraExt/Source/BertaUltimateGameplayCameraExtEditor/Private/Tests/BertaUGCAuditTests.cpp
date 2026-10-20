#include "Audit/BertaUGCAudit.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Camera/Data/UGC_CameraData.h"
#include "Camera/UGC_PlayerCameraManager.h"
#include "Components/BertaUGCCameraCycleComponent.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/InheritableComponentHandler.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/PlayerController.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

namespace
{
	UBlueprint* CreateFixture(UClass* Parent)
	{
		return FKismetEditorUtilities::CreateBlueprint(Parent, GetTransientPackage(),
			MakeUniqueObjectName(GetTransientPackage(), UBlueprint::StaticClass(), TEXT("BertaUGCAuditFixture")), BPTYPE_Normal);
	}

	void CompileFixture(UBlueprint* Blueprint)
	{
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipSave);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaUGCAuditDefaultInheritanceTest,
	"BertaDevKit.UGC.Editor.Audit.DefaultInheritance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaUGCAuditDefaultInheritanceTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UBlueprint> Parent(CreateFixture(AUGC_PlayerCameraManager::StaticClass()));
	if (!TestNotNull(TEXT("Transient manager Blueprint"), Parent.Get())) return false;
	FEdGraphPinType Type;
	Type.PinCategory = UEdGraphSchema_K2::PC_Object;
	Type.PinSubCategoryObject = UUGC_CameraDataAssetBase::StaticClass();
	if (!TestTrue(TEXT("Add exact hard-reference contract field"),
		FBlueprintEditorUtils::AddMemberVariable(Parent.Get(), TEXT("DefaultCameraData"), Type))) return false;
	CompileFixture(Parent.Get());
	if (!TestTrue(TEXT("Parent compiled"), Parent->IsUpToDate())) return false;
	const FObjectProperty* Field = FindFProperty<FObjectProperty>(Parent->GeneratedClass, TEXT("DefaultCameraData"));
	if (!TestNotNull(TEXT("Reflected field"), Field)) return false;
	TStrongObjectPtr<UUGC_CameraDataAssetBase> First(NewObject<UUGC_CameraDataAssetBase>());
	TStrongObjectPtr<UUGC_CameraDataAssetBase> Second(NewObject<UUGC_CameraDataAssetBase>());
	Field->SetObjectPropertyValue_InContainer(Parent->GeneratedClass->GetDefaultObject(), First.Get());
	TStrongObjectPtr<UBlueprint> Child(CreateFixture(Parent->GeneratedClass));
	if (!TestNotNull(TEXT("Child manager Blueprint"), Child.Get())) return false;
	CompileFixture(Child.Get());
	if (!TestTrue(TEXT("Child compiled"), Child->IsUpToDate())) return false;

	const UUGC_CameraDataAssetBase* Resolved = nullptr;
	FString Reason;
	TestTrue(TEXT("Inherited default resolves"), BertaUGCAudit::ResolveDefaultCameraData(Child->GeneratedClass, Resolved, Reason));
	TestTrue(TEXT("Reads inherited object identity"), Resolved == First.Get());
	UObject* ChildCDO = Child->GeneratedClass->GetDefaultObject();
	Field->SetObjectPropertyValue_InContainer(ChildCDO, Second.Get());
	for (int32 Run = 0; Run < 2; ++Run)
	{
		TestTrue(TEXT("Child override resolves repeatedly"), BertaUGCAudit::ResolveDefaultCameraData(Child->GeneratedClass, Resolved, Reason));
		TestTrue(TEXT("Effective child CDO wins over native/parent CDO"), Resolved == Second.Get());
	}
	TestTrue(TEXT("Audit preserves parent default"), Field->GetObjectPropertyValue_InContainer(Parent->GeneratedClass->GetDefaultObject()) == First.Get());
	TestTrue(TEXT("Audit preserves child default"), Field->GetObjectPropertyValue_InContainer(ChildCDO) == Second.Get());
	Field->SetObjectPropertyValue_InContainer(ChildCDO, nullptr);
	TestTrue(TEXT("Unassigned default is resolved, not an error"), BertaUGCAudit::ResolveDefaultCameraData(Child->GeneratedClass, Resolved, Reason));
	TestNull(TEXT("Unassigned default"), Resolved);
	TestTrue(TEXT("Null is not inconclusive"), Reason.IsEmpty());
	TestFalse(TEXT("Native manager without known field is inconclusive"), BertaUGCAudit::ResolveDefaultCameraData(AUGC_PlayerCameraManager::StaticClass(), Resolved, Reason));
	TestFalse(TEXT("Missing field supplies a reason"), Reason.IsEmpty());
	TestFalse(TEXT("Non-UGC class is not interpreted"), BertaUGCAudit::ResolveDefaultCameraData(APlayerController::StaticClass(), Resolved, Reason));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBertaUGCAuditComponentInheritanceTest,
	"BertaDevKit.UGC.Editor.Audit.ComponentInheritance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBertaUGCAuditComponentInheritanceTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UUGC_CameraDataAssetBase> First(NewObject<UUGC_CameraDataAssetBase>());
	TStrongObjectPtr<UUGC_CameraDataAssetBase> Second(NewObject<UUGC_CameraDataAssetBase>());
	TStrongObjectPtr<UBlueprint> Parent(CreateFixture(APlayerController::StaticClass()));
	if (!TestNotNull(TEXT("Transient controller Blueprint"), Parent.Get())) return false;
	USCS_Node* Node = Parent->SimpleConstructionScript->CreateNode(UBertaUGCCameraCycleComponent::StaticClass(), TEXT("CameraCycle"));
	Parent->SimpleConstructionScript->AddNode(Node);
	CastChecked<UBertaUGCCameraCycleComponent>(Node->ComponentTemplate)->CameraPresets = { First.Get(), Second.Get() };
	CompileFixture(Parent.Get());
	if (!TestTrue(TEXT("Parent compiled"), Parent->IsUpToDate())) return false;
	FString Reason;
	const UBertaUGCCameraCycleComponent* Direct = BertaUGCAudit::ResolveCycleTemplate(Parent->GeneratedClass, Reason);
	if (!TestNotNull(TEXT("Direct SCS declaration resolves"), Direct)) return false;
	if (!TestEqual(TEXT("Parent fixture preset count"), Direct->CameraPresets.Num(), 2)) return false;
	TStrongObjectPtr<UBlueprint> Child(CreateFixture(Parent->GeneratedClass));
	if (!TestNotNull(TEXT("Child controller Blueprint"), Child.Get())) return false;
	CompileFixture(Child.Get());
	if (!TestTrue(TEXT("Child controller compiled"), Child->IsUpToDate())) return false;
	TestTrue(TEXT("Unmodified child uses parent template"), BertaUGCAudit::ResolveCycleTemplate(Child->GeneratedClass, Reason) == Direct);
	Node = Parent->SimpleConstructionScript->FindSCSNode(TEXT("CameraCycle"));
	if (!TestNotNull(TEXT("Parent SCS node retained"), Node)) return false;
	UBlueprintGeneratedClass* ChildClass = CastChecked<UBlueprintGeneratedClass>(Child->GeneratedClass);
	UInheritableComponentHandler* Handler = ChildClass->GetInheritableComponentHandler(true);
	if (!TestNotNull(TEXT("Fixture override handler"), Handler)) return false;
	UBertaUGCCameraCycleComponent* Override = Cast<UBertaUGCCameraCycleComponent>(Handler->CreateOverridenComponentTemplate(FComponentKey(Node)));
	if (!TestNotNull(TEXT("Fixture inherited override"), Override)) return false;
	Override->CameraPresets = { Second.Get(), First.Get() };
	const UBertaUGCCameraCycleComponent* Effective = BertaUGCAudit::ResolveCycleTemplate(ChildClass, Reason);
	if (!TestNotNull(TEXT("Inherited override resolves"), Effective)) return false;
	if (!TestEqual(TEXT("Child fixture preset count"), Effective->CameraPresets.Num(), 2)) return false;
	TestTrue(TEXT("Child override wins"), Effective == Override);
	TestTrue(TEXT("Child array order is effective"), Effective->CameraPresets[0].Get() == Second.Get());
	const bool bDirtyBefore = Child->GetOutermost()->IsDirty();
	for (int32 Run = 0; Run < 2; ++Run)
		TestTrue(TEXT("Repeated resolution preserves template identity"), BertaUGCAudit::ResolveCycleTemplate(ChildClass, Reason) == Effective);
	TestEqual(TEXT("Resolution does not dirty package"), Child->GetOutermost()->IsDirty(), bDirtyBefore);
	TestTrue(TEXT("Parent array unchanged"), Direct->CameraPresets[0].Get() == First.Get());
	TestTrue(TEXT("Child array unchanged"), Override->CameraPresets[0].Get() == Second.Get());
	Child->Status = BS_Dirty;
	TestNull(TEXT("Uncompiled child does not fall back to parent"), BertaUGCAudit::ResolveCycleTemplate(ChildClass, Reason));
	TestFalse(TEXT("Uncompiled class supplies an inconclusive reason"), Reason.IsEmpty());
	TestNull(TEXT("Absent native component is optional"), BertaUGCAudit::ResolveCycleTemplate(APlayerController::StaticClass(), Reason));
	TestTrue(TEXT("Absence is not inconclusive"), Reason.IsEmpty());
	return true;
}

#endif
