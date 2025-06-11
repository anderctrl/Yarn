#include "YarnDialogueNodeFactory.h"
#include "YarnDialogueGraphNode_Base.h"
#include "YarnDialogueGraphNode_Branch.h"
#include "YarnDialogueGraphNode_Line.h"
#include "Slate/SGraphNode_YarnDialogue.h"
#include "Slate/SGraphNode_YarnDialogueBranch.h"
#include "Slate/SGraphNode_YarnDialogueLine.h"

TSharedPtr<SGraphNode> FYarnDialogueNodeFactory::CreateNode(UEdGraphNode* Node) const
{
	if (UYarnDialogueGraphNode_Branch* YarnNode = Cast<UYarnDialogueGraphNode_Branch>(Node))
	{
		return SNew(SGraphNode_YarnDialogueBranch, YarnNode);
	}
	if (UYarnDialogueGraphNode_Line* YarnNode = Cast<UYarnDialogueGraphNode_Line>(Node))
	{
		return SNew(SGraphNode_YarnDialogueLine, YarnNode);
	}
	if (UYarnDialogueGraphNode_Line* YarnNode = Cast<UYarnDialogueGraphNode_Line>(Node))
	{
		return SNew(SGraphNode_YarnDialogueLine, YarnNode);
	}
	if (UYarnDialogueGraphNode_Base* YarnNode = Cast<UYarnDialogueGraphNode_Base>(Node))
	{
		return SNew(SGraphNode_YarnDialogue, YarnNode);
	}
	return nullptr;
}
