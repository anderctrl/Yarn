#pragma once

#include "CoreMinimal.h"
#include "Editor/UnrealEd/Public/EdGraphUtilities.h"

class FYarnDialogueNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<class SGraphNode> CreateNode(class UEdGraphNode* Node) const override;
};
