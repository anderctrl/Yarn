#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UYarnDialogueGraphNode_Base;

class SGraphNode_YarnDialogue : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_YarnDialogue) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYarnDialogueGraphNode_Base* InNode);

	virtual void UpdateGraphNode() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

protected:
	TSharedPtr<SVerticalBox> LeftPinsBox;
	TSharedPtr<SVerticalBox> RightPinsBox;
	FSlateColor GetBackgroundColor() const;
};
