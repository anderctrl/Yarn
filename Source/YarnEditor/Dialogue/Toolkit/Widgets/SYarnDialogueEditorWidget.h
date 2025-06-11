#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FDialogueBranch;
struct FDialogueLine;
class UYarnDialogueAsset;

class SYarnDialogueEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SYarnDialogueEditorWidget)
		{
		}

		SLATE_ARGUMENT(UYarnDialogueAsset*, YarnAsset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void RebuildUI();

private:
	void OnAddBranch();
	void OnRemoveBranch(FName BranchName);

	UYarnDialogueAsset* Asset = nullptr;
	TSharedPtr<SVerticalBox> BranchesBox;
};
