#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UYarnDialogueCondition;
class UYarnDialogueGraphNode;
class UYarnDialogueGraphNode_Branch;

class SGraphNode_YarnDialogueBranch : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_YarnDialogueBranch) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYarnDialogueGraphNode_Branch* InNode);

	virtual void UpdateGraphNode() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

protected:
	TSharedPtr<SVerticalBox> LeftPinsBox;
	TSharedPtr<SVerticalBox> RightPinsBox;
	FSlateColor GetBackgroundColor() const;

	UYarnDialogueGraphNode_Branch* YarnNode = nullptr;

	TSharedRef<ITableRow> OnGenerateRowForList(UYarnDialogueCondition* Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedPtr< SListView<UYarnDialogueCondition*> > ListView;
	TArray<UYarnDialogueCondition*> Items;
};
