#include "YarnDialogueGraphSchema.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "ToolMenus.h"

void UYarnDialogueGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
}

const FPinConnectionResponse UYarnDialogueGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
    return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Read-only connection"));
}

void UYarnDialogueGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
}

FLinearColor UYarnDialogueGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
    return FLinearColor::White;
}
