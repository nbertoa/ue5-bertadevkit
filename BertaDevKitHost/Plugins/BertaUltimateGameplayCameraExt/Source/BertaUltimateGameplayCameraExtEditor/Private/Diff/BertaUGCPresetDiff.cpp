#include "Diff/BertaUGCPresetDiff.h"

#include "Camera/Data/UGC_CameraData.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogBertaUGCPresetDiff, Log, All);

namespace
{
	constexpr int32 MaxObjectDepth = 4;
	struct FDiffCounts
	{
		int32 Confirmed = 0;
		int32 Truncated = 0;
	};

	FString ExportValue(const FProperty& Property, const void* Value)
	{
		FString Text;
		Property.ExportTextItem_Direct(Text, Value, nullptr, nullptr, PPF_None);
		Text.ReplaceInline(TEXT("\r"), TEXT(" "));
		Text.ReplaceInline(TEXT("\n"), TEXT(" "));
		return Text.IsEmpty() ? TEXT("<empty>") : Text;
	}

	void Emit(const FString& Field, const FString& Left, const FString& Right, FDiffCounts& Counts)
	{
		UE_LOG(LogBertaUGCPresetDiff, Display, TEXT("[UGC Preset Diff] %s | %s | %s"), *Field, *Left, *Right);
		++Counts.Confirmed;
	}

	void CompareProperty(const FProperty& Property, const void* Left, const void* Right,
		const FString& Field, int32 Depth, FDiffCounts& Counts);

	void CompareFields(const UStruct& Struct, const void* Left, const void* Right,
		const FString& Prefix, int32 Depth, FDiffCounts& Counts, bool bEditableOnly)
	{
		for (TFieldIterator<FProperty> It(&Struct); It; ++It)
		{
			const FProperty& Property = **It;
			if (bEditableOnly && !Property.HasAnyPropertyFlags(CPF_Edit)) continue;
			const FString Field = Prefix.IsEmpty() ? Property.GetName() : Prefix + TEXT(".") + Property.GetName();
			CompareProperty(Property, Property.ContainerPtrToValuePtr<void>(Left),
				Property.ContainerPtrToValuePtr<void>(Right), Field, Depth, Counts);
		}
	}

	void CompareInstancedObject(const FObjectPropertyBase& Property, const void* Left, const void* Right,
		const FString& Field, int32 Depth, FDiffCounts& Counts)
	{
		const UObject* LeftObject = Property.GetObjectPropertyValue(Left);
		const UObject* RightObject = Property.GetObjectPropertyValue(Right);
		if (LeftObject == RightObject) return;
		if (!LeftObject || !RightObject || LeftObject->GetClass() != RightObject->GetClass())
		{
			Emit(Field, LeftObject ? LeftObject->GetClass()->GetPathName() : TEXT("<null>"),
				RightObject ? RightObject->GetClass()->GetPathName() : TEXT("<null>"), Counts);
			return;
		}
		if (Depth >= MaxObjectDepth)
		{
			UE_LOG(LogBertaUGCPresetDiff, Display,
				TEXT("[UGC Preset Diff] %s | <comparison truncated at instanced-object depth %d>"), *Field, MaxObjectDepth);
			++Counts.Truncated;
			return;
		}
		const int32 BeforeConfirmed = Counts.Confirmed;
		const int32 BeforeTruncated = Counts.Truncated;
		CompareFields(*LeftObject->GetClass(), LeftObject, RightObject, Field, Depth + 1, Counts, true);
		if (BeforeConfirmed == Counts.Confirmed && BeforeTruncated == Counts.Truncated && LeftObject->GetPathName() != RightObject->GetPathName())
		{
			// Different owned instances with equal editable settings are not a settings difference.
			UE_LOG(LogBertaUGCPresetDiff, Verbose, TEXT("[UGC Preset Diff] %s has distinct instances with equal reflected editable settings."), *Field);
		}
	}

	void CompareProperty(const FProperty& Property, const void* Left, const void* Right,
		const FString& Field, int32 Depth, FDiffCounts& Counts)
	{
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
		{
			if (!Property.Identical(Left, Right))
			{
				// Only reflected member differences count; exporting a struct can expose instanced-object Outer paths.
				CompareFields(*StructProperty->Struct, Left, Right, Field, Depth, Counts, false);
			}
			return;
		}
		if (const FArrayProperty* Array = CastField<FArrayProperty>(&Property))
		{
			FScriptArrayHelper LeftArray(Array, const_cast<void*>(Left));
			FScriptArrayHelper RightArray(Array, const_cast<void*>(Right));
			if (LeftArray.Num() != RightArray.Num())
				Emit(Field + TEXT(".Num"), FString::FromInt(LeftArray.Num()), FString::FromInt(RightArray.Num()), Counts);
			for (int32 Index = 0; Index < FMath::Min(LeftArray.Num(), RightArray.Num()); ++Index)
			{
				CompareProperty(*Array->Inner, LeftArray.GetRawPtr(Index), RightArray.GetRawPtr(Index),
					FString::Printf(TEXT("%s[%d]"), *Field, Index), Depth, Counts);
			}
			for (int32 Index = LeftArray.Num(); Index < RightArray.Num(); ++Index)
				Emit(FString::Printf(TEXT("%s[%d]"), *Field, Index), TEXT("<missing>"),
					ExportValue(*Array->Inner, RightArray.GetRawPtr(Index)), Counts);
			for (int32 Index = RightArray.Num(); Index < LeftArray.Num(); ++Index)
				Emit(FString::Printf(TEXT("%s[%d]"), *Field, Index),
					ExportValue(*Array->Inner, LeftArray.GetRawPtr(Index)), TEXT("<missing>"), Counts);
			return;
		}
		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(&Property))
		{
			if (Property.HasAnyPropertyFlags(CPF_InstancedReference))
			{
				CompareInstancedObject(*ObjectProperty, Left, Right, Field, Depth, Counts);
				return;
			}
			const UObject* LeftObject = ObjectProperty->GetObjectPropertyValue(Left);
			const UObject* RightObject = ObjectProperty->GetObjectPropertyValue(Right);
			const FString LeftPath = LeftObject ? LeftObject->GetPathName() : TEXT("<null>");
			const FString RightPath = RightObject ? RightObject->GetPathName() : TEXT("<null>");
			if (LeftPath != RightPath) Emit(Field, LeftPath, RightPath, Counts);
			return;
		}
		if (!Property.Identical(Left, Right))
			Emit(Field, ExportValue(Property, Left), ExportValue(Property, Right), Counts);
	}
}

void BertaUGCPresetDiff::Run(const UUGC_CameraDataAssetBase& Left, const UUGC_CameraDataAssetBase& Right)
{
	UE_LOG(LogBertaUGCPresetDiff, Display, TEXT("[UGC Preset Diff] Field | %s | %s"), *Left.GetPathName(), *Right.GetPathName());
	FDiffCounts Counts;
	if (Left.GetClass() != Right.GetClass())
	{
		Emit(TEXT("Class"), Left.GetClass()->GetPathName(), Right.GetClass()->GetPathName(), Counts);
	}
	for (TFieldIterator<FProperty> It(UUGC_CameraDataAssetBase::StaticClass()); It; ++It)
	{
		const FProperty& Property = **It;
		if (Property.GetOwnerStruct() != UUGC_CameraDataAssetBase::StaticClass()) continue;
		CompareProperty(Property, Property.ContainerPtrToValuePtr<void>(&Left),
			Property.ContainerPtrToValuePtr<void>(&Right), Property.GetName(), 0, Counts);
	}
	UE_LOG(LogBertaUGCPresetDiff, Display,
		TEXT("[UGC Preset Diff] Complete: %d confirmed difference(s); %d instanced path(s) not fully inspected due to max depth. UGC base properties are compared; subclass-only fields and curve content are outside this pass. Object references and curves are compared by path."),
		Counts.Confirmed, Counts.Truncated);
}
