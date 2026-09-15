#pragma once

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphSchema.h"
#include "ObjectGraph/BertaObjectGraphSnapshot.h"

#include "BertaObjectGraphEdGraph.generated.h"

UCLASS(Transient)
class UBertaObjectGraphEdGraph : public UEdGraph
{
	GENERATED_BODY()
};

UCLASS(Transient)
class UBertaObjectGraphEdNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	int32 SnapshotId = INDEX_NONE;
	FString CopiedName;
	FString CopiedClass;
	FString CopiedPath;
	BertaObjectGraph::ERole Role = BertaObjectGraph::ERole::Intermediate;

	virtual void AllocateDefaultPins() override {}
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
};

UCLASS(Transient)
class UBertaObjectGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override {}
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("This reference graph is read-only."));
	}
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
};
