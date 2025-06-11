#include "YarnDialogueGraphNode_Line.h"

#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"

void UYarnDialogueGraphNode_Line::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, TEXT("Exec"), TEXT("In"));
	CreatePin(EGPD_Output, TEXT("Exec"), TEXT("Out"));
}

UEdGraphPin* UYarnDialogueGraphNode_Line::GetOutputPin() const
{
	return FindPin(TEXT("Out"));
}

UEdGraphPin* UYarnDialogueGraphNode_Line::GetInputPin() const
{
	return FindPin(TEXT("In"));
}

FText UYarnDialogueGraphNode_Line::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!DialogueAsset)
		return FText::FromString(TEXT("Invalid Line"));

	if (const FDialogueLine* Line = DialogueAsset->GetLineByID(LineID))
	{
		return Line->Text;
	}
	if (const FDialogueLine* Choice = DialogueAsset->GetChoiceByID(LineID))
	{
		return Choice->Text;
	}

	return FText::FromName(LineID);
}

FLinearColor UYarnDialogueGraphNode_Line::GetNodeBodyTintColor() const
{
	if (DialogueAsset->GetLineByID(LineID))
	{
		return FLinearColor(0.f, 0.f, 1.f, 1.f);
	}
	if (DialogueAsset->GetChoiceByID(LineID))
	{
		return FLinearColor(0.f, 1.f, 0.f, 1.f);
	}
	return FLinearColor::Gray;
}

