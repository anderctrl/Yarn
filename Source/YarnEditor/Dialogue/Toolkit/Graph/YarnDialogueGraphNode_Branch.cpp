#include "YarnDialogueGraphNode_Branch.h"
#include "EdGraphSchema_K2.h"

void UYarnDialogueGraphNode_Branch::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, TEXT("MultipleNodes"), TEXT("In"));

	if (!DialogueAsset)
		return;

	const FDialogueBranch* Branch = DialogueAsset->GetBranchByID(BranchID);
	if (!Branch)
		return;

	CreatePin(EGPD_Output, TEXT("MultipleNodes"), TEXT("Out"));
}

FText UYarnDialogueGraphNode_Branch::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(DialogueAsset->GetBranchByID(BranchID)->DisplayName);
}

UEdGraphPin* UYarnDialogueGraphNode_Branch::GetOutputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction == EGPD_Output)
		{
			return Pin;
		}
	}
	return nullptr;
}

UEdGraphPin* UYarnDialogueGraphNode_Branch::GetInputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction == EGPD_Input)
			return Pin;
	}
	return nullptr;
}

FLinearColor UYarnDialogueGraphNode_Branch::GetNodeBodyTintColor() const
{
	return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
}
