#include "AI/BertaBlackboardDebugUtils.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"

namespace BertaBlackboardDebugUtilsPrivate
{
struct FKeyDescription
{
	FName Name;
	FString Type;
	FString Value;
};
}

bool UBertaBlackboardDebugUtils::GetDebugSummary(
	const UBlackboardComponent* Blackboard,
	FString& OutSummary)
{
	OutSummary.Reset();
	if (!IsValid(Blackboard))
	{
		return false;
	}

	const UBlackboardData* BlackboardAsset = Blackboard->GetBlackboardAsset();
	if (!BlackboardAsset)
	{
		return false;
	}

	TArray<BertaBlackboardDebugUtilsPrivate::FKeyDescription> KeyDescriptions;
	const int32 KeyCount = BlackboardAsset->GetNumKeys();
	KeyDescriptions.Reserve(KeyCount);
	for (int32 KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
	{
		const FBlackboard::FKey KeyId = static_cast<FBlackboard::FKey>(KeyIndex);
		const FName KeyName = Blackboard->GetKeyName(KeyId);
		const TSubclassOf<UBlackboardKeyType> KeyType = Blackboard->GetKeyType(KeyId);
		if (KeyName.IsNone() || !KeyType)
		{
			continue;
		}

		BertaBlackboardDebugUtilsPrivate::FKeyDescription& Description = KeyDescriptions.AddDefaulted_GetRef();
		Description.Name = KeyName;
		Description.Type = KeyType->GetName();
		Description.Value = Blackboard->DescribeKeyValue(KeyId, EBlackboardDescription::OnlyValue);
	}

	KeyDescriptions.Sort([](
		const BertaBlackboardDebugUtilsPrivate::FKeyDescription& Left,
		const BertaBlackboardDebugUtilsPrivate::FKeyDescription& Right)
	{
		return Left.Name.LexicalLess(Right.Name);
	});

	TArray<FString> Lines;
	Lines.Reserve(KeyDescriptions.Num() + 2);
	Lines.Add(FString::Printf(TEXT("Blackboard Asset: %s"), *BlackboardAsset->GetName()));
	Lines.Add(FString::Printf(TEXT("Keys: %d"), KeyDescriptions.Num()));
	for (const BertaBlackboardDebugUtilsPrivate::FKeyDescription& Description : KeyDescriptions)
	{
		Lines.Add(FString::Printf(
			TEXT("%s [%s] = %s"),
			*Description.Name.ToString(),
			*Description.Type,
			*Description.Value));
	}

	OutSummary = FString::Join(Lines, TEXT("\n"));
	return true;
}

bool UBertaBlackboardDebugUtils::GetKeyValueDescription(
	const UBlackboardComponent* Blackboard,
	FName KeyName,
	FString& OutDescription)
{
	OutDescription.Reset();
	if (!IsValid(Blackboard) || KeyName.IsNone())
	{
		return false;
	}

	const FBlackboard::FKey KeyId = Blackboard->GetKeyID(KeyName);
	if (KeyId == FBlackboard::InvalidKey)
	{
		return false;
	}

	OutDescription = Blackboard->DescribeKeyValue(KeyId, EBlackboardDescription::OnlyValue);
	return true;
}
