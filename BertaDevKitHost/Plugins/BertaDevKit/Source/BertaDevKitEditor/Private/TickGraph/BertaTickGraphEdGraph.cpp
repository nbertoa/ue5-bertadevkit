#include "TickGraph/BertaTickGraphEdGraph.h"

#include "Styling/AppStyle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BertaTickGraphEdGraph)

FText UBertaTickGraphEdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(CopiedName + TEXT("\n") + CopiedKind + TEXT(" | ") + CopiedGroup
		+ (bActive ? TEXT("\nEnabled · Registered") : TEXT("\nDisabled or Unregistered")));
}

FText UBertaTickGraphEdNode::GetTooltipText() const
{
	return FText::FromString(CopiedPath + TEXT("\nPrerequisite → Dependent; configured relation, not a frame trace."));
}

FLinearColor UBertaTickGraphEdNode::GetNodeTitleColor() const
{
	if (!bActive) { return FAppStyle::Get().GetColor(TEXT("Colors.Foreground")); }
	switch (Role)
	{
	case BertaTickGraph::ERole::TargetActorPrimary: return FAppStyle::Get().GetColor(TEXT("Colors.AccentGreen"));
	case BertaTickGraph::ERole::TargetComponentPrimary: return FAppStyle::Get().GetColor(TEXT("Colors.AccentBlue"));
	case BertaTickGraph::ERole::CustomPrerequisite: return FAppStyle::Get().GetColor(TEXT("Colors.AccentYellow"));
	default: return FAppStyle::Get().GetColor(TEXT("Colors.Foreground"));
	}
}

FLinearColor UBertaTickGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FAppStyle::Get().GetColor(TEXT("Colors.AccentBlue"));
}
