#include "SGraphNode_YarnDialogue.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "SGraphPin.h"
#include "Dialogue/Toolkit/Graph/YarnDialogueGraphNode_Base.h"

void SGraphNode_YarnDialogue::Construct(const FArguments& InArgs, UYarnDialogueGraphNode_Base* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
}

void SGraphNode_YarnDialogue::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center).AttachWidget(SNullWidget::NullWidget);

	this->GetOrAddSlot(ENodeZone::Center)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
		.Padding(2)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
			.Padding(2)
			.BorderBackgroundColor(this, &SGraphNode_YarnDialogue::GetBackgroundColor)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 6)
				[
					SNew(STextBlock)
					.Text(GraphNode ? GraphNode->GetNodeTitle(ENodeTitleType::FullTitle) : FText::FromString("Invalid"))
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
			]
		]
	];

	CreatePinWidgets();
}

void SGraphNode_YarnDialogue::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

FSlateColor SGraphNode_YarnDialogue::GetBackgroundColor() const
{
	return GraphNode->GetNodeBodyTintColor();
}
