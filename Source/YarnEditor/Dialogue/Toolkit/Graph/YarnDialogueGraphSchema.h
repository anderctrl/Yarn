#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "YarnDialogueGraphSchema.generated.h"

UCLASS()
class YARNEDITOR_API UYarnDialogueGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:

	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;

	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
};