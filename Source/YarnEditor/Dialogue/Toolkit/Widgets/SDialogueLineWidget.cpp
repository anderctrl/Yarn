#include "SDialogueLineWidget.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "PropertyCustomizationHelpers.h"
#include "SDialogueChoiceWidget.h"
#include "Yarn/YarnSettings.h"

#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"
#include "Yarn/Dialogue/Data/YarnDialogueEvent.h"
#include "Yarn/Dialogue/Data/YarnDialogueSpeaker.h"

bool SDialogueLineWidget::IsPlayerSpeaker() const
{
	return Asset->GetLineByID(LineID)->SpeakerID == "Player";
}

bool SDialogueLineWidget::HasMultipleChoices() const
{
	if (!IsValidAsset()) return false;
	const FDialogueLine* CurrentLine = Asset->GetLineByID(LineID);
	return (CurrentLine->PlayerChoiceIDs.Num() > 0);
}

void SDialogueLineWidget::Construct(const FArguments& InArgs)
{
	LineID = InArgs._LineID;
	BranchID = InArgs._BranchID;
	Asset = InArgs._Asset;

	RefreshUI();
}

void SDialogueLineWidget::RefreshUI()
{
	if (!IsValidAsset()) return;

	BranchOptions.Empty();
	BranchOptions.Insert(MakeShared<FName>(NAME_None), 0);

	const FDialogueBranch* CurrentBranch = Asset->GetBranchByID(BranchID);
	if (!CurrentBranch) return;

	const FName CurrentJumpTo = Asset->GetLineByID(LineID)->JumpToBranchID;

	for (const FDialogueBranch& Branch : Asset->Branches)
	{
		TSharedPtr<FName> BranchIDPtr = MakeShared<FName>(Branch.BranchID);
		BranchOptions.Add(BranchIDPtr);
		if (Branch.BranchID == CurrentJumpTo)
		{
			CurrentSelectedJumpTo = BranchIDPtr;
		}
	}

	SpeakerOptions.Empty();
	SpeakerOptions.Add(MakeShared<FName>(NAME_None));

	UDialogueSpeakerList* SpeakerList = UYarnSettings::Get()->GetLoadedSpeakerList();
	const FName CurrentSpeakerID = Asset->GetLineByID(LineID)->SpeakerID;

	if (SpeakerList)
	{
		for (const FDialogueSpeaker& Speaker : SpeakerList->Speakers)
		{
			TSharedPtr<FName> IDPtr = MakeShared<FName>(Speaker.SpeakerID);
			SpeakerOptions.Add(IDPtr);

			if (Speaker.SpeakerID == CurrentSpeakerID)
			{
				CurrentSelectedSpeaker = IDPtr;
			}
		}
	}

	if (SpeakerComboBox.IsValid()) SpeakerComboBox->RefreshOptions();
	if (JumpToComboBox.IsValid()) JumpToComboBox->RefreshOptions();

	PlayerChoicesUI = GenerateChoicesUI();
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
			.VAlign(VAlign_Top)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.MinHeight(30.f)
					.MaxHeight(30.f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
						.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.MinWidth(50)
							.FillWidth(0.2f)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.Padding(5, 0, 0, 0)
								.VAlign(VAlign_Center)
								[
									SNew(SBorder)
									.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
									.BorderBackgroundColor(FSlateColor(EStyleColor::AccentBlack))
									[
										SNew(SComboBox<TSharedPtr<FName>>)
										.OptionsSource(&SpeakerOptions)
										.ComboBoxStyle(FAppStyle::Get(), "SimpleComboBox")
										.HasDownArrow(false)
										.OnGenerateWidget(this, &SDialogueLineWidget::GenerateSpeakerItem)
										.OnSelectionChanged(this, &SDialogueLineWidget::OnSpeakerSelectionChanged)
										.InitiallySelectedItem(CurrentSelectedSpeaker)
										[
											SNew(STextBlock)
											.Text(this, &SDialogueLineWidget::GetCurrentSpeakerDisplayName)
										]
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(5, 0)
								.VAlign(VAlign_Center)
								[
									SNew(SImage)
									.DesiredSizeOverride(FVector2D(16.f, 16.f))
									.Image(
										FSlateIcon(FName("EditorStyle"), "DialogueWaveDetails.SpeakerToTarget").
										GetIcon())
								]
							]

							+ SHorizontalBox::Slot()
							.FillWidth(0.8f)
							.Padding(0)
							.VAlign(VAlign_Center)
							.MinWidth(100)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								[
									SNew(SBox)
									[
										SNew(SEditableTextBox)
										.Text(this, &SDialogueLineWidget::GetLineText)
										.OnTextCommitted(this, &SDialogueLineWidget::OnTextCommitted)
									]
								]
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
								.ObjectPath(this, &SDialogueLineWidget::GetCurrentSoundPath)
								.OnObjectChanged(this, &SDialogueLineWidget::OnSoundSelected)
							]
						]
					]
					+ SVerticalBox::Slot()
					.Padding(0, 5, 0, 0)
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
								.BorderBackgroundColor(this, &SDialogueLineWidget::GetJumpToBorderColor)
								[
									SAssignNew(JumpToComboBox, SComboBox<TSharedPtr<FName>>)
									.OptionsSource(&BranchOptions)
									.OnGenerateWidget(this, &SDialogueLineWidget::GenerateBranchComboItem)
									.OnSelectionChanged(this, &SDialogueLineWidget::OnJumpToSelectionChanged)
									.InitiallySelectedItem(CurrentSelectedJumpTo)
									[
										SNew(STextBlock)
										.Text(this, &SDialogueLineWidget::GetCurrentJumpToText)
									]
								]
							]
						]
					]

					+ SVerticalBox::Slot()
					.Padding(0, 5, 0, 0)
					.HAlign(HAlign_Fill)
					.AutoHeight()
					[
						SNew(SBorder)
						.Padding(0)
						.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
						.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
						.Visibility(this, &SDialogueLineWidget::GetChoicesVisibility)
						[
							PlayerChoicesUI.ToSharedRef()
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
									.SelectedClass(this, &SDialogueLineWidget::GetCurrentEventClass)
									.OnSetClass(this, &SDialogueLineWidget::OnEventClassSelected)
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
		]
	];
}

TSharedRef<SWidget> SDialogueLineWidget::GenerateChoicesUI()
{
	if (!IsValidAsset() || IsPlayerSpeaker())
	{
		return SNullWidget::NullWidget;
	}

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	TSharedRef<SExpandableArea> ExpandableArea =
		SNew(SExpandableArea)
		.InitiallyCollapsed(false)
		.Padding(0)
		.BodyBorderBackgroundColor(FSlateColor(EStyleColor::Background))
		.HeaderContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Player choices"))
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			.Padding(0)
			[
				SNew(SButton)
				.Text(FText::FromString("Add Choice"))
				.OnClicked_Lambda([this]()
				{
					if (!IsValidAsset()) return FReply::Handled();

					FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "AddChoice", "Add Player Choice"));
					Asset->Modify();

					FName NewID;
					int32 Suffix = 1;
					do
					{
						NewID = FName(*FString::Printf(TEXT("%s_Choice_%d"), *LineID.ToString(), Suffix++));
					}
					while (Asset->GetChoiceByID(NewID) != nullptr);

					FDialogueLine NewLine;
					NewLine.LineID = NewID;
					NewLine.SpeakerID = FName("Player");
					NewLine.Text = FText::FromString("New Choice");

					Asset->GetBranchByID(BranchID)->PlayerChoices.Add(NewLine);
					Asset->GetLineByID(LineID)->PlayerChoiceIDs.Add(NewID);
					Asset->MarkPackageDirty();
					RefreshUI();

					return FReply::Handled();
				})
			]
		]
		.BodyContent()
		[
			Box
		];


	for (const auto Element : Asset->GetChoicesForLineID(LineID))
	{
		FName ChoiceID = Element.LineID;
		Box->AddSlot()
		   .AutoHeight()
		   .Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SDialogueChoiceWidget)
				.BranchID(BranchID)
				.ChoiceID(ChoiceID)
				.Asset(Asset)
			]
			+ SHorizontalBox::Slot()
			.Padding(5, 0, 0, 0)
			.VAlign(VAlign_Top)
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked_Lambda([this, ChoiceID]()
				{
					OnRemoveChoice(ChoiceID);
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FSlateIcon(FName("EditorStyle"), "MyBlueprint.DeleteEntry").GetIcon())
				]
			]
		];
	}

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
		.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
		[
			ExpandableArea
		];
}

void SDialogueLineWidget::OnRemoveChoice(FName ChoiceID)
{
	FDialogueBranch* Branch = Asset->GetBranchByID(BranchID);
	if (!Branch || !Branch->PlayerChoices.Contains(ChoiceID)) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "RemoveChoice", "Remove Player Choice"));
	Asset->Modify();

	Branch->PlayerChoices.Remove(ChoiceID);
	Asset->GetLineByID(LineID)->PlayerChoiceIDs.Remove(ChoiceID);
	Asset->MarkPackageDirty();
	RefreshUI();
}

EVisibility SDialogueLineWidget::GetJumpToVisibility() const
{
	return HasMultipleChoices() ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SDialogueLineWidget::GetChoicesVisibility() const
{
	if (!IsValidAsset()) return EVisibility::Collapsed;

	return IsPlayerSpeaker() ? EVisibility::Collapsed : EVisibility::Visible;
}

void SDialogueLineWidget::RefreshBranchOptions()
{
	if (!IsValidAsset()) return;

	const FName CurrentJumpTo = Asset->GetLineByID(LineID)->JumpToBranchID;

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

	const UYarnSettings* Settings = UYarnSettings::Get();
	UDialogueSpeakerList* SpeakerList = Settings ? Settings->GetLoadedSpeakerList() : nullptr;

	SpeakerOptions.Empty();
	SpeakerOptions.Add(MakeShared<FName>(NAME_None));

	const FName CurrentSpeakerID = Asset->GetLineByID(LineID)->SpeakerID;

	for (const FDialogueSpeaker& Speaker : SpeakerList ? SpeakerList->Speakers : TArray<FDialogueSpeaker>())
	{
		TSharedPtr<FName> IDPtr = MakeShared<FName>(Speaker.SpeakerID);
		SpeakerOptions.Add(IDPtr);

		if (Speaker.SpeakerID == CurrentSpeakerID)
		{
			CurrentSelectedSpeaker = IDPtr;
		}
	}

	if (!CurrentSelectedSpeaker.IsValid())
	{
		CurrentSelectedSpeaker = SpeakerOptions[0];
	}

	if (SpeakerComboBox.IsValid())
	{
		SpeakerComboBox->RefreshOptions();
	}
}

TSharedRef<IDetailsView> SDialogueLineWidget::CreateEventDetailsView()
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
		DetailsView->SetObject(Asset->GetLineByID(LineID)->Event);
	}

	return DetailsView;
}

FString SDialogueLineWidget::GetCurrentSoundPath() const
{
	if (Asset->GetLineByID(LineID)->Audio)
	{
		return Asset->GetLineByID(LineID)->Audio->GetPathName();
	}
	return FString();
}

void SDialogueLineWidget::OnSoundSelected(const FAssetData& AssetData)
{
	if (!IsValidAsset())
		return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "SetDialogueSound", "Set Dialogue Audio"));
	Asset->Modify();

	USoundBase* SelectedSound = Cast<USoundBase>(AssetData.GetAsset());
	Asset->GetLineByID(LineID)->Audio = SelectedSound;

	Asset->MarkPackageDirty();
	RefreshUI();
}

const UClass* SDialogueLineWidget::GetCurrentEventClass() const
{
	if (!IsValidAsset()) return nullptr;
	return Asset->GetLineByID(LineID)->Event ? Asset->GetLineByID(LineID)->Event->GetClass() : nullptr;
}

void SDialogueLineWidget::OnEventClassSelected(const UClass* NewClass)
{
	if (NewClass && (!IsValidAsset() || !NewClass->IsChildOf(UYarnDialogueEvent::StaticClass())))
		return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "SetDialogueEvent", "Set Dialogue Event"));
	Asset->Modify();

	UYarnDialogueEvent* NewEvent;


	if (NewClass == nullptr)
	{
		Asset->GetLineByID(LineID)->Event = nullptr;
		NewEvent = nullptr;
	}
	else
	{
		NewEvent = NewObject<UYarnDialogueEvent>(Asset, NewClass, NAME_None, RF_Transactional);
		Asset->GetLineByID(LineID)->Event = NewEvent;
	}


	Asset->MarkPackageDirty();
	RefreshUI();

	if (EventDetailsView.IsValid())
	{
		EventDetailsView->SetObject(NewEvent);
	}
}

TSharedRef<SWidget> SDialogueLineWidget::GenerateSpeakerItem(TSharedPtr<FName> InItem) const
{
	const UDialogueSpeakerList* SpeakerList = UYarnSettings::Get()->GetLoadedSpeakerList();
	if (SpeakerList)
	{
		const FDialogueSpeaker* Speaker = SpeakerList->FindSpeakerByID(*InItem);
		if (Speaker)
		{
			return SNew(STextBlock).Text(FText::FromString(Speaker->DisplayName.ToString()));
		}
	}
	return SNew(STextBlock).Text(FText::FromName(*InItem));
}

void SDialogueLineWidget::OnSpeakerSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid() || !IsValidAsset()) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "SetSpeakerID", "Set Speaker ID"));
	Asset->Modify();

	Asset->GetLineByID(LineID)->SpeakerID = *NewSelection;

	Asset->MarkPackageDirty();
	RefreshUI();
	CurrentSelectedSpeaker = NewSelection;
}


FText SDialogueLineWidget::GetCurrentSpeakerDisplayName() const
{
	if (!CurrentSelectedSpeaker.IsValid())
		return FText::FromString("None");

	const UDialogueSpeakerList* SpeakerList = UYarnSettings::Get()->GetLoadedSpeakerList();
	if (SpeakerList)
	{
		const FDialogueSpeaker* Speaker = SpeakerList->FindSpeakerByID(*CurrentSelectedSpeaker);
		if (Speaker)
		{
			return FText::FromString(Speaker->DisplayName.ToString());
		}
	}
	return FText::FromName(*CurrentSelectedSpeaker);
}

FText SDialogueLineWidget::GetLineText() const
{
	if (!IsValidAsset()) return FText::GetEmpty();
	return Asset->GetLineByID(LineID)->Text;
}

void SDialogueLineWidget::OnTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (!IsValidAsset()) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "EditText", "Edit Text"));
	Asset->Modify();

	FString Namespace = TEXT("DialogueNamespace");
	FString Key = LineID.ToString();

	FText LocalizedText = FText::ChangeKey(Namespace, Key, NewText);

	Asset->GetLineByID(LineID)->Text = LocalizedText;
	Asset->MarkPackageDirty();
	RefreshUI();
}

TSharedRef<SWidget> SDialogueLineWidget::GenerateBranchComboItem(TSharedPtr<FName> Item) const
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

void SDialogueLineWidget::OnJumpToSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid() || !IsValidAsset()) return;

	FScopedTransaction Transaction(NSLOCTEXT("YarnEditor", "SetJumpTo", "Set Jump Target"));
	Asset->Modify();
	Asset->GetLineByID(LineID)->JumpToBranchID = *NewSelection;
	Asset->MarkPackageDirty();
	RefreshUI();

	CurrentSelectedJumpTo = NewSelection;
}

FText SDialogueLineWidget::GetCurrentJumpToText() const
{
	if (!IsValidAsset()) return FText::GetEmpty();

	const FDialogueBranch* CurrentBranch = Asset->GetBranchByID(BranchID);
	if (!CurrentBranch) return FText::GetEmpty();

	const FName JumpToID = Asset->GetLineByID(LineID)->JumpToBranchID;
	if (JumpToID.IsNone())
		return FText::FromString("None");

	const FDialogueBranch* JumpToBranch = Asset->GetBranchByID(JumpToID);
	if (!JumpToBranch)
		return FText::FromName(JumpToID);
	if (JumpToBranch->BranchID == UYarnDialogueAsset::GetMainBranchID())
		return FText::FromString("Main");

	return FText::FromString(JumpToBranch->DisplayName);
}

FSlateColor SDialogueLineWidget::GetJumpToBorderColor() const
{
	if (!IsValidAsset()) return FSlateColor(EStyleColor::Background);

	FSlateColor NormalColor = FSlateColor(EStyleColor::Background);
	if (Asset->GetLineByID(LineID)->JumpToBranchID == NAME_None)
	{
		return NormalColor;
	}
	return IsJumpTargetValid() ? NormalColor : FSlateColor(EStyleColor::Error);
}

void SDialogueLineWidget::RebuildEventPropertyWidget()
{
	if (!IsValidAsset()) return;

	UYarnDialogueEvent* EventObject = Asset->GetLineByID(LineID)->Event;

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

bool SDialogueLineWidget::IsValidAsset() const
{
	return Asset && Asset->GetBranchByID(BranchID);
}

bool SDialogueLineWidget::IsJumpTargetValid() const
{
	return Asset->GetBranchByID(Asset->GetLineByID(LineID)->JumpToBranchID) != nullptr;
}
