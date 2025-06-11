#include "SGraphNode_YarnDialogueBranch.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "SGraphPin.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"
#include "Yarn/Dialogue/Data/YarnDialogueCondition.h"
#include "YarnEditor/Dialogue/Toolkit/Graph/YarnDialogueGraphNode_Branch.h"

void SGraphNode_YarnDialogueBranch::Construct(const FArguments& InArgs, UYarnDialogueGraphNode_Branch* InNode)
{
	GraphNode = InNode;
	YarnNode = InNode;
	UpdateGraphNode();
}

void SGraphNode_YarnDialogueBranch::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	if (!YarnNode || !YarnNode->DialogueAsset)
	{
		return;
	}

	const FDialogueBranch* DialogueBranch;
	if (YarnNode->DialogueAsset->GetBranchByID(YarnNode->BranchID))
	{
		DialogueBranch = YarnNode->DialogueAsset->GetBranchByID(YarnNode->BranchID);

		Items.Append(DialogueBranch->EnterConditions);
	}
	else
	{
		DialogueBranch = nullptr;
	}

	FText BranchNameText = FText::FromString(DialogueBranch->DisplayName);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center).AttachWidget(SNullWidget::NullWidget);

	this->GetOrAddSlot(ENodeZone::Center)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
			.Padding(2)
			.BorderBackgroundColor(this, &SGraphNode_YarnDialogueBranch::GetBackgroundColor)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.Text(BranchNameText)
					.Font(FAppStyle::GetFontStyle("BoldFont"))
					.Justification(ETextJustify::Center)
					.ShadowOffset(FVector2D(1, 1))
					.ShadowColorAndOpacity(FLinearColor::Black)
				]

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("NoBrush"))
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SAssignNew(LeftPinsBox, SVerticalBox)
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							SNew(SSpacer)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Right)
						[
							SAssignNew(RightPinsBox, SVerticalBox)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					Items.IsEmpty()
						? SNullWidget::NullWidget
						: SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
						.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
						.Padding(5)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							[
								SNew(STextBlock)
								.Text(FText::FromString("Conditions:"))
								.Font(FAppStyle::GetFontStyle("BoldFont"))
								.Justification(ETextJustify::Center)
								.ShadowOffset(FVector2D(1, 1))
								.ShadowColorAndOpacity(FLinearColor::Black)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(5)
							[
								SNew(SListView<UYarnDialogueCondition*>)
								.ListItemsSource(&Items)
								.SelectionMode(ESelectionMode::None)
								.IsFocusable(false)
								.OnGenerateRow(this, &SGraphNode_YarnDialogueBranch::OnGenerateRowForList)
							]
						]

				]
			]
		]
	];

	CreatePinWidgets();
}

TSharedRef<ITableRow> SGraphNode_YarnDialogueBranch::OnGenerateRowForList(
	UYarnDialogueCondition* Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
			SNew(STableRow<UYarnDialogueCondition*>, OwnerTable)
			[
				SNew(STextBlock)
				.Text(FText::FromString(GetNameSafe(Item)))
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(1, 1))
				.ShadowColorAndOpacity(FLinearColor::Black)
			];
}

void SGraphNode_YarnDialogueBranch::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	if (const UEdGraphPin* PinObj = PinToAdd->GetPinObj(); PinObj && PinObj->bAdvancedView)
	{
		PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
	}

	TSharedPtr<SVerticalBox> PinBox;
	if (PinToAdd->GetDirection() == EGPD_Input)
	{
		PinBox = LeftPinsBox;
		InputPins.Add(PinToAdd);
	}
	else
	{
		PinBox = RightPinsBox;
		OutputPins.Add(PinToAdd);
	}

	if (PinBox)
	{
		PinBox->AddSlot()
		      .HAlign(HAlign_Center)
		      .VAlign(VAlign_Center)
		[
			SNew(SBox)
			[
				PinToAdd
			]
		];
	}
}

FSlateColor SGraphNode_YarnDialogueBranch::GetBackgroundColor() const
{
	return GraphNode->GetNodeBodyTintColor();
}
