#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UYarnDialogueGraphNode;
class UYarnDialogueGraphNode_Line;

class SGraphNode_YarnDialogueLine : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_YarnDialogueLine) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYarnDialogueGraphNode_Line* InNode);

	virtual void UpdateGraphNode() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

protected:
	TSharedPtr<SVerticalBox> LeftPinsBox;
	TSharedPtr<SVerticalBox> RightPinsBox;
	FSlateColor GetBackgroundColor() const;

	UYarnDialogueGraphNode_Line* YarnNode = nullptr;
};
