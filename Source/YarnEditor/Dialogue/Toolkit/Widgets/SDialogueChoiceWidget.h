#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"

class UYarnDialogueAsset;

class SDialogueChoiceWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogueChoiceWidget)
		{
		}

		SLATE_ARGUMENT(FName, ChoiceID)
		SLATE_ARGUMENT(FName, BranchID)
		SLATE_ARGUMENT(UYarnDialogueAsset*, Asset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void RefreshUI();
	void RefreshBranchOptions();

private:
	FName BranchID = NAME_None;
	FName ChoiceID = NAME_None;
	UYarnDialogueAsset* Asset = nullptr;

	TSharedPtr<SComboBox<TSharedPtr<FName>>> JumpToComboBox;
	TArray<TSharedPtr<FName>> BranchOptions;
	TSharedPtr<FName> CurrentSelectedJumpTo;

	const UClass* CurrentSelectedEventClass = nullptr;
	TSharedPtr<IDetailsView> EventDetailsView;

	FText GetLineText() const;
	void OnTextCommitted(const FText& NewText, ETextCommit::Type CommitType);

	TSharedRef<SWidget> GenerateBranchComboItem(TSharedPtr<FName> Item) const;
	void OnJumpToSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo);
	FText GetCurrentJumpToText() const;
	FSlateColor GetJumpToBorderColor() const;
	void RebuildEventPropertyWidget();

	void OnEventClassSelected(const UClass* NewClass);
	const UClass* GetCurrentEventClass() const;
	TSharedRef<IDetailsView> CreateEventDetailsView();

	FString GetCurrentSoundPath() const;
	void OnSoundSelected(const FAssetData& AssetData);

	bool IsJumpTargetValid() const;
	bool IsValidAsset() const;
};

class USpeakerList;