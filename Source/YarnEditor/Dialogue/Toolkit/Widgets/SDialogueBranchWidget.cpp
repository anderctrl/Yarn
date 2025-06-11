#include "SDialogueBranchWidget.h"

#include "PropertyCustomizationHelpers.h"
#include "SDialogueLineWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"
#include "Yarn/Dialogue/Data/YarnDialogueCondition.h"

FOnBranchListChanged SDialogueBranchWidget::OnBranchListChanged;

SDialogueBranchWidget::~SDialogueBranchWidget()
{
	OnBranchListChanged.RemoveAll(this);
}

void SDialogueBranchWidget::Construct(const FArguments& InArgs)
{
	BranchID = InArgs._BranchID;
	Asset = InArgs._Asset;
	AdditionalContent = InArgs._AdditionalContent.Widget;

	OnBranchListChanged.AddSP(SharedThis(this), &SDialogueBranchWidget::OnBranchListUpdated);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FSlateColor(EStyleColor::Header))
		.Padding(5)
		[
			SNew(SExpandableArea)
			.HeaderContent()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("NoBorder"))
				[
					SNew(SOverlay)

					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0, 0, 5, 0)
							[
								SNew(SBox).WidthOverride(16.f).HeightOverride(16.f)
								[
									SNew(SImage)
									.Image(FSlateIcon(FName("EditorStyle"), "GraphEditor.Branch_16x").GetIcon())
								]
							]

							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.MaxWidth(200.f)
								.MinWidth(200.f)
								[
									SAssignNew(BranchNameEditable, SEditableTextBox)
									.Text(FText::FromString(Asset->GetBranchByID(BranchID)->DisplayName))
									.OnTextCommitted(this, &SDialogueBranchWidget::OnBranchNameCommitted)
									.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
								]
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(5, 0)
							[
								SNew(SButton)
								.Text(FText::FromString("Add Line"))
								.OnClicked_Lambda([this]()
								{
									OnAddLine();
									return FReply::Handled();
								})
							]
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					.Padding(5, 0)
					[
						SNew(SBox)
						[
							BranchID == FName(UYarnDialogueAsset::GetMainBranchID())
								? SNew(STextBlock)
								.Text(FText::FromString("MAIN"))
								.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
								.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
								: AdditionalContent.IsValid()
								? AdditionalContent.ToSharedRef()
								: SNullWidget::NullWidget

						]
					]
				]
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					BuildConditionsWidget()
				]
				+ SVerticalBox::Slot()
				.Padding(0, 5, 0, 0)
				.AutoHeight()
				[
					SAssignNew(LineListView, SListView<TSharedPtr<int32>>)
					.ListItemsSource(&LineIndices)
					.IsFocusable(false)
					.SelectionMode(ESelectionMode::None)
					.OnGenerateRow(this, &SDialogueBranchWidget::OnGenerateLineRow)

				]
				+ SVerticalBox::Slot()
				.Padding(0, 5, 0, 0)
				.MaxHeight(25.f)
				.MinHeight(25.f)
				[
					SNew(SButton)
					.Text(FText::FromString("Add Line"))
					.OnClicked_Lambda([this]()
					{
						OnAddLine();
						return FReply::Handled();
					})
				]
			]
		]
	];

	RefreshConditions();
	RefreshLines();
}

TSharedRef<SWidget> SDialogueBranchWidget::BuildConditionsWidget()
{
	if (BranchID == FName(UYarnDialogueAsset::GetMainBranchID()))
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
		.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(false)
			.Padding(0)
			.BodyBorderBackgroundColor(FSlateColor(EStyleColor::Background))
			.HeaderContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Branch Enter Conditions"))
				]
				+ SHorizontalBox::Slot()
				.Padding(5, 0)
				.FillWidth(0.8f)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.MaxWidth(100.f)
					[
						SNew(SClassPropertyEntryBox)
						.MetaClass(UYarnDialogueCondition::StaticClass())
						.AllowNone(true)
						.AllowAbstract(false)
						.SelectedClass(this, &SDialogueBranchWidget::GetCurrentConditionClass)
						.OnSetClass(this, &SDialogueBranchWidget::OnConditionClassSelected)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(FText::FromString("Add Condition"))
						.OnClicked_Lambda([this]()
						{
							OnAddCondition();
							return FReply::Handled();
						})
					]
				]
			]
			.BodyContent()
			[
				SAssignNew(ConditionListView, SListView<TSharedPtr<int32>>)
				.ListItemsSource(&ConditionIndices)
				.OnGenerateRow(this, &SDialogueBranchWidget::OnGenerateConditionRow)
				.SelectionMode(ESelectionMode::None)
				.IsFocusable(false)
			]
		];
}


TSharedRef<ITableRow> SDialogueBranchWidget::OnGenerateLineRow(
	TSharedPtr<int32> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch || !Item.IsValid() || *Item < 0 || *Item >= Branch->Lines.Num())
	{
		return SNew(STableRow<TSharedPtr<int32>>, OwnerTable);
	}

	const FDialogueLine* Line = &Branch->Lines[*Item];
	if (!Line)
	{
		return SNew(STableRow<TSharedPtr<int32>>, OwnerTable);
	}

	TSharedRef<SDialogueLineWidget> LineWidget =
		SNew(SDialogueLineWidget)
		.BranchID(BranchID)
		.LineID(Line->LineID)
		.Asset(Asset);

	DialogueLineWidgets.Add(LineWidget);

	return SNew(STableRow<TSharedPtr<int32>>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(5)
			[
				LineWidget
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			.Padding(0, 5, 5, 0)
			[
				SNew(SButton)
				.OnClicked_Lambda([this, LineID = Line->LineID]()
				{
					OnRemoveLine(LineID);
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FSlateIcon(FName("EditorStyle"), "MyBlueprint.DeleteEntry").GetIcon())
				]
			]
		];
}

void SDialogueBranchWidget::OnAddCondition()
{
	if (!Asset || !SelectedConditionClass)
		return;

	FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "AddCondition", "Add Dialogue Condition"));
	Asset->Modify();

	UYarnDialogueCondition* NewCondition = NewObject<UYarnDialogueCondition>(
		Asset, SelectedConditionClass, NAME_None, RF_Transactional);
	Branch->EnterConditions.Add(NewCondition);

	Asset->MarkPackageDirty();
	RefreshConditions();
}

void SDialogueBranchWidget::OnRemoveCondition(int32 Index)
{
	FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch || !Branch->EnterConditions.IsValidIndex(Index)) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "RemoveCondition", "Remove Dialogue Condition"));
	Asset->Modify();

	Branch->EnterConditions.RemoveAt(Index);
	Asset->MarkPackageDirty();
	RefreshConditions();
}


void SDialogueBranchWidget::OnBranchNameCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (!Asset) return;

	FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch) return;

	FString NewName = NewText.ToString();
	if (NewName.IsEmpty() || NewName == Branch->DisplayName) return;

	for (auto Element : Asset->Branches)
	{
		if (Element.DisplayName == NewName)
		{
			return;
		}
	}

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "RenameBranch", "Rename Dialogue Branch"));
	Asset->Modify();

	Branch->DisplayName = NewName;
	OnBranchListChanged.Broadcast();
	Asset->MarkPackageDirty();
}


void SDialogueBranchWidget::OnBranchListUpdated()
{
	for (const TSharedPtr<SDialogueLineWidget>& Widget : DialogueLineWidgets)
	{
		if (Widget.IsValid())
		{
			Widget->RefreshBranchOptions();
		}
	}
}

TSharedRef<ITableRow> SDialogueBranchWidget::OnGenerateConditionRow(
	TSharedPtr<int32> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	int32 Index = *Item;
	UYarnDialogueCondition* Condition = nullptr;

	if (Asset && Asset->GetBranchByID(BranchID))
	{
		FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
		if (Branch->EnterConditions.IsValidIndex(Index))
		{
			Condition = Branch->EnterConditions[Index];
		}
	}

	TSharedRef<IDetailsView> ConditionDetails = CreateConditionDetailsView(Condition);
	ConditionDetailsViews.Add(ConditionDetails);

	return SNew(STableRow<TSharedPtr<int32>>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString(GetNameSafe(Condition)))
				]
				+ SVerticalBox::Slot()
				.Padding(0, 5, 0, 0)
				.AutoHeight()
				[
					ConditionDetails
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(5)
			.VAlign(VAlign_Top)
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked_Lambda([this, Index]()
				{
					OnRemoveCondition(Index);
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FSlateIcon(FName("EditorStyle"), "MyBlueprint.DeleteEntry").GetIcon())
				]
			]
		];
}


TSharedRef<IDetailsView> SDialogueBranchWidget::CreateConditionDetailsView(UObject* Object)
{
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bAllowSearch = false;
	Args.bShowScrollBar = false;
	Args.bLockable = false;
	Args.bShowPropertyMatrixButton = false;
	Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	TSharedRef<IDetailsView> DetailsView = PropertyEditor.CreateDetailView(Args);
	DetailsView->SetObject(Object);
	return DetailsView;
}

void SDialogueBranchWidget::RefreshConditions()
{
	if (!Asset) return;

	FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch) return;

	ConditionIndices.Empty();
	ConditionDetailsViews.Empty();

	for (int32 i = 0; i < Branch->EnterConditions.Num(); ++i)
	{
		ConditionIndices.Add(MakeShared<int32>(i));
	}

	if (ConditionListView.IsValid())
	{
		ConditionListView->RequestListRefresh();
	}
}

void SDialogueBranchWidget::OnConditionClassSelected(const UClass* NewClass)
{
	if (!NewClass || !NewClass->IsChildOf(UYarnDialogueCondition::StaticClass()))
		return;

	SelectedConditionClass = NewClass;
}

const UClass* SDialogueBranchWidget::GetCurrentConditionClass() const
{
	return SelectedConditionClass;
}

void SDialogueBranchWidget::RefreshLines()
{
	LineIndices.Empty();
	DialogueLineWidgets.Empty();

	if (!Asset)
		return;

	const FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch)
		return;

	for (int32 Index = 0; Index < Branch->Lines.Num(); ++Index)
	{
		LineIndices.Add(MakeShared<int32>(Index));
	}

	if (LineListView.IsValid())
	{
		LineListView->RequestListRefresh();
	}
}

void SDialogueBranchWidget::OnAddLine()
{
	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "AddLine", "Add Dialogue Line"));
	Asset->Modify();

	FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch) return;


	FName NewID;
	int32 Suffix = 1;
	do
	{
		NewID = FName(*FString::Printf(TEXT("%s_Line_%d"), *BranchID.ToString(), Suffix++));
	}
	while (Asset->GetLineByID(NewID) != nullptr);


	Branch->Lines.Add(FDialogueLine(NewID));
	Asset->MarkPackageDirty();
	RefreshLines();
}


void SDialogueBranchWidget::OnRemoveLine(FName LineID)
{
	FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch || !Branch->Lines.Contains(LineID)) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "RemoveLine", "Remove Dialogue Line"));
	Asset->Modify();

	Branch->Lines.Remove(LineID);
	for (auto Element : Asset->GetChoicesForLineID(LineID))
	{
		Branch->PlayerChoices.Remove(Element.LineID);
	}
	Asset->MarkPackageDirty();
	RefreshLines();
}
