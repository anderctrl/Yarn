#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"

class UYarnDialogueAsset;

class SDialogueLineWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogueLineWidget)
		{
		}

		SLATE_ARGUMENT(FName, LineID)
		SLATE_ARGUMENT(FName, BranchID)
		SLATE_ARGUMENT(UYarnDialogueAsset*, Asset)
	SLATE_END_ARGS()

	bool IsPlayerSpeaker() const;
	bool HasMultipleChoices() const;
	void Construct(const FArguments& InArgs);
	void RefreshUI();
	EVisibility GetJumpToVisibility() const;
	EVisibility GetChoicesVisibility() const;
	void RefreshBranchOptions();

private:
	FName BranchID = NAME_None;
	FName LineID = NAME_None;
	UYarnDialogueAsset* Asset = nullptr;

	TSharedPtr<SComboBox<TSharedPtr<FName>>> SpeakerComboBox;
	TArray<TSharedPtr<FName>> SpeakerOptions;
	TSharedPtr<FName> CurrentSelectedSpeaker;

	TSharedPtr<SComboBox<TSharedPtr<FName>>> JumpToComboBox;
	TArray<TSharedPtr<FName>> BranchOptions;
	TSharedPtr<FName> CurrentSelectedJumpTo;

	const UClass* CurrentSelectedEventClass = nullptr;
	TSharedPtr<IDetailsView> EventDetailsView;

	TSharedPtr<SWidget> PlayerChoicesUI;

	TSharedRef<SWidget> GenerateSpeakerItem(TSharedPtr<FName> InItem) const;
	void OnSpeakerSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo);
	FText GetCurrentSpeakerDisplayName() const;

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

	TSharedRef<SWidget> GenerateChoicesUI();
	void OnRemoveChoice(FName ChoiceID);

	bool IsJumpTargetValid() const;
	bool IsValidAsset() const;
};

class USpeakerList;