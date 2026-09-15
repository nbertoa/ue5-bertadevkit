#include "ObjectGraph/BertaObjectGraphEdGraph.h"

#include "Styling/AppStyle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BertaObjectGraphEdGraph)

FText UBertaObjectGraphEdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(CopiedName + TEXT("\n") + CopiedClass);
}

FText UBertaObjectGraphEdNode::GetTooltipText() const
{
	return FText::FromString(CopiedPath);
}

FLinearColor UBertaObjectGraphEdNode::GetNodeTitleColor() const
{
	switch (Role)
	{
	case BertaObjectGraph::ERole::Root:
		return FAppStyle::Get().GetColor(TEXT("Colors.AccentBlue"));
	case BertaObjectGraph::ERole::Target:
		return FAppStyle::Get().GetColor(TEXT("Colors.AccentGreen"));
	default:
		return FAppStyle::Get().GetColor(TEXT("Colors.Foreground"));
	}
}

FLinearColor UBertaObjectGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FAppStyle::Get().GetColor(TEXT("Colors.AccentBlue"));
}
