#pragma once

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphSchema.h"
#include "TickGraph/BertaTickGraphSnapshot.h"

#include "BertaTickGraphEdGraph.generated.h"

UCLASS(Transient)
class UBertaTickGraphEdGraph : public UEdGraph
{
	GENERATED_BODY()
};

UCLASS(Transient)
class UBertaTickGraphEdNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	int32 SnapshotId = INDEX_NONE;
	FString CopiedName;
	FString CopiedKind;
	FString CopiedGroup;
	FString CopiedPath;
	bool bActive = false;
	BertaTickGraph::ERole Role = BertaTickGraph::ERole::ExternalPrerequisite;

	virtual void AllocateDefaultPins() override {}
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
};

UCLASS(Transient)
class UBertaTickGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override {}
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Tick Graph is read-only."));
	}
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
};
