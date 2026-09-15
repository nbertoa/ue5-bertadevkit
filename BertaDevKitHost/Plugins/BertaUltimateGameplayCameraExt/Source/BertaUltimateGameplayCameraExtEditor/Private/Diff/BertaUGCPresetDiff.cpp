#include "Diff/BertaUGCPresetDiff.h"

#include "Camera/Data/UGC_CameraData.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaUGCPresetDiff, Log, All);

namespace
{
	constexpr int32 MaxObjectDepth = 4;

	FString ExportValue(const FProperty& Property, const void* Value)
	{
		FString Text;
		Property.ExportTextItem_Direct(Text, Value, nullptr, nullptr, PPF_None);
		Text.ReplaceInline(TEXT("\r"), TEXT(" "));
		Text.ReplaceInline(TEXT("\n"), TEXT(" "));
		return Text.IsEmpty() ? TEXT("<empty>") : Text;
	}

	void Emit(const FString& Field, const FString& Left, const FString& Right, int32& ChangeCount)
	{
		UE_LOG(LogBertaUGCPresetDiff, Display, TEXT("[UGC Preset Diff] %s | %s | %s"), *Field, *Left, *Right);
		++ChangeCount;
	}

	void CompareProperty(const FProperty& Property, const void* Left, const void* Right,
		const FString& Field, int32 Depth, int32& ChangeCount);

	void CompareFields(const UStruct& Struct, const void* Left, const void* Right,
		const FString& Prefix, int32 Depth, int32& ChangeCount, bool bEditableOnly)
	{
		for (TFieldIterator<FProperty> It(&Struct); It; ++It)
		{
			const FProperty& Property = **It;
			if (bEditableOnly && !Property.HasAnyPropertyFlags(CPF_Edit)) continue;
			const FString Field = Prefix.IsEmpty() ? Property.GetName() : Prefix + TEXT(".") + Property.GetName();
			CompareProperty(Property, Property.ContainerPtrToValuePtr<void>(Left),
				Property.ContainerPtrToValuePtr<void>(Right), Field, Depth, ChangeCount);
		}
	}

	void CompareInstancedObject(const FObjectPropertyBase& Property, const void* Left, const void* Right,
		const FString& Field, int32 Depth, int32& ChangeCount)
	{
		const UObject* LeftObject = Property.GetObjectPropertyValue(Left);
		const UObject* RightObject = Property.GetObjectPropertyValue(Right);
		if (LeftObject == RightObject) return;
		if (!LeftObject || !RightObject || LeftObject->GetClass() != RightObject->GetClass() || Depth >= MaxObjectDepth)
		{
			Emit(Field, LeftObject ? LeftObject->GetClass()->GetPathName() : TEXT("<null>"),
				RightObject ? RightObject->GetClass()->GetPathName() : TEXT("<null>"), ChangeCount);
			return;
		}
		const int32 Before = ChangeCount;
		CompareFields(*LeftObject->GetClass(), LeftObject, RightObject, Field, Depth + 1, ChangeCount, true);
		if (Before == ChangeCount && LeftObject->GetPathName() != RightObject->GetPathName())
		{
			// Different owned instances with equal editable settings are not a settings difference.
			UE_LOG(LogBertaUGCPresetDiff, Verbose, TEXT("[UGC Preset Diff] %s has distinct instances with equal reflected editable settings."), *Field);
		}
	}

	void CompareProperty(const FProperty& Property, const void* Left, const void* Right,
		const FString& Field, int32 Depth, int32& ChangeCount)
	{
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
		{
			if (!Property.Identical(Left, Right))
			{
				const int32 Before = ChangeCount;
				CompareFields(*StructProperty->Struct, Left, Right, Field, Depth, ChangeCount, false);
				if (Before == ChangeCount) Emit(Field, ExportValue(Property, Left), ExportValue(Property, Right), ChangeCount);
			}
			return;
		}
		if (const FArrayProperty* Array = CastField<FArrayProperty>(&Property))
		{
			FScriptArrayHelper LeftArray(Array, const_cast<void*>(Left));
			FScriptArrayHelper RightArray(Array, const_cast<void*>(Right));
			if (LeftArray.Num() != RightArray.Num())
				Emit(Field + TEXT(".Num"), FString::FromInt(LeftArray.Num()), FString::FromInt(RightArray.Num()), ChangeCount);
			for (int32 Index = 0; Index < FMath::Min(LeftArray.Num(), RightArray.Num()); ++Index)
			{
				CompareProperty(*Array->Inner, LeftArray.GetRawPtr(Index), RightArray.GetRawPtr(Index),
					FString::Printf(TEXT("%s[%d]"), *Field, Index), Depth, ChangeCount);
			}
			for (int32 Index = LeftArray.Num(); Index < RightArray.Num(); ++Index)
				Emit(FString::Printf(TEXT("%s[%d]"), *Field, Index), TEXT("<missing>"),
					ExportValue(*Array->Inner, RightArray.GetRawPtr(Index)), ChangeCount);
			for (int32 Index = RightArray.Num(); Index < LeftArray.Num(); ++Index)
				Emit(FString::Printf(TEXT("%s[%d]"), *Field, Index),
					ExportValue(*Array->Inner, LeftArray.GetRawPtr(Index)), TEXT("<missing>"), ChangeCount);
			return;
		}
		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(&Property))
		{
			if (Property.HasAnyPropertyFlags(CPF_InstancedReference))
			{
				CompareInstancedObject(*ObjectProperty, Left, Right, Field, Depth, ChangeCount);
				return;
			}
			const UObject* LeftObject = ObjectProperty->GetObjectPropertyValue(Left);
			const UObject* RightObject = ObjectProperty->GetObjectPropertyValue(Right);
			const FString LeftPath = LeftObject ? LeftObject->GetPathName() : TEXT("<null>");
			const FString RightPath = RightObject ? RightObject->GetPathName() : TEXT("<null>");
			if (LeftPath != RightPath) Emit(Field, LeftPath, RightPath, ChangeCount);
			return;
		}
		if (!Property.Identical(Left, Right))
			Emit(Field, ExportValue(Property, Left), ExportValue(Property, Right), ChangeCount);
	}
}

void BertaUGCPresetDiff::Run(const UUGC_CameraDataAssetBase& Left, const UUGC_CameraDataAssetBase& Right)
{
	UE_LOG(LogBertaUGCPresetDiff, Display, TEXT("[UGC Preset Diff] Field | %s | %s"), *Left.GetPathName(), *Right.GetPathName());
	int32 ChangeCount = 0;
	for (TFieldIterator<FProperty> It(UUGC_CameraDataAssetBase::StaticClass()); It; ++It)
	{
		const FProperty& Property = **It;
		if (Property.GetOwnerStruct() != UUGC_CameraDataAssetBase::StaticClass()) continue;
		CompareProperty(Property, Property.ContainerPtrToValuePtr<void>(&Left),
			Property.ContainerPtrToValuePtr<void>(&Right), Property.GetName(), 0, ChangeCount);
	}
	UE_LOG(LogBertaUGCPresetDiff, Display,
		TEXT("[UGC Preset Diff] Complete: %d differing UGC property path(s). Object references and curves are compared by path; custom subclass-only fields and curve content are outside this pass."), ChangeCount);
}
