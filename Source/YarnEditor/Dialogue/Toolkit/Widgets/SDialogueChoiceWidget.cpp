#include "SDialogueChoiceWidget.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "PropertyCustomizationHelpers.h"

#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"
#include "Yarn/Dialogue/Data/YarnDialogueEvent.h"

void SDialogueChoiceWidget::Construct(const FArguments& InArgs)
{
	ChoiceID = InArgs._ChoiceID;
	BranchID = InArgs._BranchID;
	Asset = InArgs._Asset;

	RefreshUI();
}

void SDialogueChoiceWidget::RefreshUI()
{
	if (!IsValidAsset()) return;

	BranchOptions.Empty();
	BranchOptions.Insert(MakeShared<FName>(NAME_None), 0);

	const FDialogueBranch* CurrentBranch = Asset->GetBranchByID(BranchID);
	if (!CurrentBranch) return;

	const FName CurrentJumpTo = Asset->GetChoiceByID(ChoiceID)->JumpToBranchID;

	for (const FDialogueBranch& Branch : Asset->Branches)
	{
		TSharedPtr<FName> BranchIDPtr = MakeShared<FName>(Branch.BranchID);
		BranchOptions.Add(BranchIDPtr);
		if (Branch.BranchID == CurrentJumpTo)
		{
			CurrentSelectedJumpTo = BranchIDPtr;
		}
	}

	if (JumpToComboBox.IsValid()) JumpToComboBox->RefreshOptions();

	EventDetailsView = CreateEventDetailsView();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(5)
		.BorderBackgroundColor(FSlateColor(EStyleColor::AccentBlack))
		.BorderImage(FCoreStyle::Get().GetBrush("ColorPicker.RoundedSolidBackground"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(0)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
					.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.MinWidth(50.f)
						.FillWidth(0.2f)
						.Padding(5, 0, 0, 0)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString("Text"))
						]

						+ SHorizontalBox::Slot()
						.FillWidth(0.8f)
						.MinWidth(100)
						[
							SNew(SEditableTextBox)
							.Text(this, &SDialogueChoiceWidget::GetLineText)
							.OnTextCommitted(this, &SDialogueChoiceWidget::OnTextCommitted)
						]
					]
				]
				+ SVerticalBox::Slot()
				.MinHeight(30.f)
				.MaxHeight(30.f)
				.Padding(0, 5, 0, 0)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
					.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.MinWidth(50.f)
						.FillWidth(0.2f)
						.Padding(5, 0, 0, 0)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString("Audio"))
						]

						+ SHorizontalBox::Slot()
						.FillWidth(0.8f)
						.MinWidth(100)
						[
							SNew(SObjectPropertyEntryBox)
							.AllowedClass(USoundBase::StaticClass())
							.AllowClear(true)
							.ObjectPath(this, &SDialogueChoiceWidget::GetCurrentSoundPath)
							.OnObjectChanged(this, &SDialogueChoiceWidget::OnSoundSelected)
						]
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.MinHeight(30.f)
				.MaxHeight(30.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
					.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.MinWidth(50.f)
						.FillWidth(0.2f)
						.Padding(5, 0, 0, 0)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString("Jumps to"))
						]

						+ SHorizontalBox::Slot()
						.FillWidth(0.8f)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
							.BorderBackgroundColor(this, &SDialogueChoiceWidget::GetJumpToBorderColor)
							[
								SAssignNew(JumpToComboBox, SComboBox<TSharedPtr<FName>>)
								.OptionsSource(&BranchOptions)
								.OnGenerateWidget(this, &SDialogueChoiceWidget::GenerateBranchComboItem)
								.OnSelectionChanged(this, &SDialogueChoiceWidget::OnJumpToSelectionChanged)
								.InitiallySelectedItem(CurrentSelectedJumpTo)
								[
									SNew(STextBlock)
									.Text(this, &SDialogueChoiceWidget::GetCurrentJumpToText)
								]
							]
						]
					]
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.Padding(10, 0)
			.VAlign(VAlign_Top)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
				.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(0.7f)
						.MinWidth(100)
						.Padding(4, 2)
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0, 0, 5, 0)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text(FText::FromString("Event"))
							]
							+ SHorizontalBox::Slot()
							[
								SNew(SClassPropertyEntryBox)
								.MetaClass(UYarnDialogueEvent::StaticClass())
								.AllowAbstract(false)
								.AllowNone(true)
								.SelectedClass(this, &SDialogueChoiceWidget::GetCurrentEventClass)
								.OnSetClass(this, &SDialogueChoiceWidget::OnEventClassSelected)
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(5)
					.AutoHeight()
					[
						SNew(SBox)
						[
							EventDetailsView.ToSharedRef()
						]
					]
				]
			]
		]
	];
}

void SDialogueChoiceWidget::RefreshBranchOptions()
{
	if (!IsValidAsset()) return;

	const FName CurrentJumpTo = Asset->GetChoiceByID(ChoiceID)->JumpToBranchID;

	BranchOptions.Empty();
	BranchOptions.Insert(MakeShared<FName>(NAME_None), 0);

	for (const FDialogueBranch& Branch : Asset->Branches)
	{
		TSharedPtr<FName> BranchIDPtr = MakeShared<FName>(Branch.BranchID);
		BranchOptions.Add(BranchIDPtr);

		if (Branch.BranchID == CurrentJumpTo)
		{
			CurrentSelectedJumpTo = BranchIDPtr;
		}
	}

	if (JumpToComboBox.IsValid())
	{
		JumpToComboBox->RefreshOptions();
	}
}

TSharedRef<IDetailsView> SDialogueChoiceWidget::CreateEventDetailsView()
{
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bAllowSearch = false;
	Args.bShowScrollBar = false;
	Args.bLockable = false;
	Args.bAllowFavoriteSystem = false;
	Args.bShowPropertyMatrixButton = false;
	Args.bShowCustomFilterOption = false;
	Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

	TSharedRef<IDetailsView> DetailsView = PropertyEditor.CreateDetailView(Args);

	if (IsValidAsset())
	{
		DetailsView->SetObject(Asset->GetChoiceByID(ChoiceID)->Event);
	}

	return DetailsView;
}

FString SDialogueChoiceWidget::GetCurrentSoundPath() const
{
	if (Asset->GetChoiceByID(ChoiceID)->Audio)
	{
		return Asset->GetChoiceByID(ChoiceID)->Audio->GetPathName();
	}
	return FString();
}

void SDialogueChoiceWidget::OnSoundSelected(const FAssetData& AssetData)
{
	if (!IsValidAsset())
		return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "SetDialogueSound", "Set Dialogue Audio"));
	Asset->Modify();

	USoundBase* SelectedSound = Cast<USoundBase>(AssetData.GetAsset());
	Asset->GetChoiceByID(ChoiceID)->Audio = SelectedSound;

	Asset->MarkPackageDirty();
	RefreshUI();
}

const UClass* SDialogueChoiceWidget::GetCurrentEventClass() const
{
	if (!IsValidAsset()) return nullptr;
	return Asset->GetChoiceByID(ChoiceID)->Event ? Asset->GetChoiceByID(ChoiceID)->Event->GetClass() : nullptr;
}

void SDialogueChoiceWidget::OnEventClassSelected(const UClass* NewClass)
{
	if (NewClass && (!IsValidAsset() || !NewClass->IsChildOf(UYarnDialogueEvent::StaticClass())))
		return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "SetDialogueEvent", "Set Dialogue Event"));
	Asset->Modify();

	UYarnDialogueEvent* NewEvent;


	if (NewClass == nullptr)
	{
		Asset->GetChoiceByID(ChoiceID)->Event = nullptr;
		NewEvent = nullptr;
	}
	else
	{
		NewEvent = NewObject<UYarnDialogueEvent>(Asset, NewClass, NAME_None, RF_Transactional);
		Asset->GetChoiceByID(ChoiceID)->Event = NewEvent;
	}


	Asset->MarkPackageDirty();
	RefreshUI();

	if (EventDetailsView.IsValid())
	{
		EventDetailsView->SetObject(NewEvent);
	}
}

FText SDialogueChoiceWidget::GetLineText() const
{
	if (!IsValidAsset()) return FText::GetEmpty();
	return Asset->GetChoiceByID(ChoiceID)->Text;
}

void SDialogueChoiceWidget::OnTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (!IsValidAsset()) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "EditText", "Edit Text"));
	Asset->Modify();

	FString Namespace = TEXT("DialogueNamespace");
	FString Key = ChoiceID.ToString();

	FText LocalizedText = FText::ChangeKey(Namespace, Key, NewText);

	Asset->GetChoiceByID(ChoiceID)->Text = LocalizedText;
	Asset->MarkPackageDirty();
	RefreshUI();
}

TSharedRef<SWidget> SDialogueChoiceWidget::GenerateBranchComboItem(TSharedPtr<FName> Item) const
{
	if (!Item.IsValid() || Item->IsNone())
	return SNew(STextBlock).Text(FText::FromString("None"));

	const FDialogueBranch* Branch = Asset->GetBranchByID(*Item);
	if (!Branch)
	return SNew(STextBlock).Text(FText::FromName(*Item));

	FString BranchName = Branch->DisplayName;
	if (Branch->BranchID == UYarnDialogueAsset::GetMainBranchID())
	{
		BranchName = "Main";
	}
	return SNew(STextBlock).Text(FText::FromString(BranchName));
}

void SDialogueChoiceWidget::OnJumpToSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid() || !IsValidAsset()) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "SetJumpTo", "Set Jump Target"));
	Asset->Modify();
	Asset->GetChoiceByID(ChoiceID)->JumpToBranchID = *NewSelection;
	Asset->MarkPackageDirty();
	RefreshUI();

	CurrentSelectedJumpTo = NewSelection;
}

FText SDialogueChoiceWidget::GetCurrentJumpToText() const
{
	if (!IsValidAsset()) return FText::GetEmpty();

	const FDialogueBranch* CurrentBranch = Asset->GetBranchByID(BranchID);
	if (!CurrentBranch) return FText::GetEmpty();

	const FName JumpToID = Asset->GetChoiceByID(ChoiceID)->JumpToBranchID;
	if (JumpToID.IsNone())
		return FText::FromString("None");

	const FDialogueBranch* JumpToBranch = Asset->GetBranchByID(JumpToID);
	if (!JumpToBranch)
		return FText::FromName(JumpToID);
	if (JumpToBranch->BranchID == UYarnDialogueAsset::GetMainBranchID())
		return FText::FromString("Main");

	return FText::FromString(JumpToBranch->DisplayName);
}

FSlateColor SDialogueChoiceWidget::GetJumpToBorderColor() const
{
	if (!IsValidAsset()) return FSlateColor(EStyleColor::Background);

	FSlateColor NormalColor = FSlateColor(EStyleColor::Background);
	if (Asset->GetChoiceByID(ChoiceID)->JumpToBranchID == NAME_None)
	{
		return NormalColor;
	}
	return IsJumpTargetValid() ? NormalColor : FSlateColor(EStyleColor::Error);
}

void SDialogueChoiceWidget::RebuildEventPropertyWidget()
{
	if (!IsValidAsset()) return;

	UYarnDialogueEvent* EventObject = Asset->GetChoiceByID(ChoiceID)->Event;

	if (!EventObject)
	{
		EventDetailsView.Reset();
		return;
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bAllowSearch = true;
	Args.bLockable = false;
	Args.bUpdatesFromSelection = false;
	Args.NotifyHook = nullptr;
	Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	EventDetailsView = PropertyModule.CreateDetailView(Args);
	if (EventDetailsView.IsValid())
	{
		EventDetailsView->SetObject(EventObject);
	}
}

bool SDialogueChoiceWidget::IsValidAsset() const
{
	return Asset && Asset->GetBranchByID(BranchID);
}

bool SDialogueChoiceWidget::IsJumpTargetValid() const
{
	return Asset->GetBranchByID(Asset->GetChoiceByID(ChoiceID)->JumpToBranchID) != nullptr;
}
