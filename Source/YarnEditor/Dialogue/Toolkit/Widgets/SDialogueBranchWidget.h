#pragma once

#include "Widgets/SCompoundWidget.h"

class SClassPropertyEntryBox;
class SDialogueLineWidget;
class UYarnDialogueAsset;
class IDetailsView;

DECLARE_MULTICAST_DELEGATE(FOnBranchListChanged);

class SDialogueBranchWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogueBranchWidget) {}
	SLATE_ARGUMENT(FName, BranchID)
	SLATE_ARGUMENT(UYarnDialogueAsset*, Asset)
	SLATE_DEFAULT_SLOT(FArguments, AdditionalContent)
SLATE_END_ARGS()

static FOnBranchListChanged OnBranchListChanged;

	virtual ~SDialogueBranchWidget() override;
	void Construct(const FArguments& InArgs);

private:
	FName BranchID = NAME_None;
	UYarnDialogueAsset* Asset = nullptr;
	TSharedPtr<SWidget> AdditionalContent = nullptr;

	TSharedPtr<SEditableTextBox> BranchNameEditable;
	TSharedPtr<SListView<TSharedPtr<int32>>> LineListView;
	TSharedPtr<SListView<TSharedPtr<int32>>> ConditionListView;

	TArray<TSharedPtr<int32>> LineIndices;
	TArray<TSharedPtr<int32>> ConditionIndices;
	TArray<TSharedPtr<SDialogueLineWidget>> DialogueLineWidgets;
	TArray<TSharedPtr<IDetailsView>> ConditionDetailsViews;

	const UClass* SelectedConditionClass = nullptr;

	TSharedRef<SWidget> BuildConditionsWidget();

	void RefreshLines();
	void OnAddLine();
	void OnRemoveLine(FName LineID);
	TSharedRef<ITableRow> OnGenerateLineRow(TSharedPtr<int32> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void RefreshConditions();
	void OnAddCondition();
	void OnRemoveCondition(int32 Index);
	TSharedRef<ITableRow> OnGenerateConditionRow(TSharedPtr<int32> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<IDetailsView> CreateConditionDetailsView(UObject* Object);
	void OnConditionClassSelected(const UClass* NewClass);
	const UClass* GetCurrentConditionClass() const;

	void OnBranchNameCommitted(const FText& NewText, ETextCommit::Type CommitType);
	void OnBranchListUpdated();
};
