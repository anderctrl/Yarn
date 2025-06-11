#include "SGraphNode_YarnDialogueLine.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "SGraphPin.h"
#include "Yarn/YarnSettings.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"
#include "Yarn/Dialogue/Data/YarnDialogueSpeaker.h"
#include "Yarn/Dialogue/Data/YarnDialogueEvent.h"
#include "YarnEditor/Dialogue/Toolkit/Graph/YarnDialogueGraphNode_Line.h"

void SGraphNode_YarnDialogueLine::Construct(const FArguments& InArgs, UYarnDialogueGraphNode_Line* InNode)
{
	GraphNode = InNode;
	YarnNode = InNode;
	UpdateGraphNode();
}

void SGraphNode_YarnDialogueLine::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	if (!YarnNode || !YarnNode->DialogueAsset)
	{
		return;
	}

	const FDialogueLine* DialogueLine;
	if (YarnNode->DialogueAsset->GetLineByID(YarnNode->LineID))
	{
		DialogueLine = YarnNode->DialogueAsset->GetLineByID(YarnNode->LineID);
	}
	else if (YarnNode->DialogueAsset->GetChoiceByID(YarnNode->LineID))
	{
		DialogueLine = YarnNode->DialogueAsset->GetChoiceByID(YarnNode->LineID);
	}
	else
	{
		DialogueLine = nullptr;
	}

	const UYarnSettings* YarnSettings = UYarnSettings::Get();
	const UDialogueSpeakerList* SpeakerList = YarnSettings ? YarnSettings->GetLoadedSpeakerList() : nullptr;

	FText DialogueText = DialogueLine->Text;
	FText EventText = FText::FromString(GetNameSafe(DialogueLine->Event));
	FText SpeakerName = FText::FromString("Invalid");
	if (SpeakerList)
	{
		if (const auto Speaker = SpeakerList->FindSpeakerByID(DialogueLine->SpeakerID))
		{
			SpeakerName = Speaker->DisplayName;
		}
	}

	FString FullText = FString::Printf(TEXT("%s: %s"), *SpeakerName.ToString(), *DialogueText.ToString());

	if (FullText.Len() > 40)
	{
		FullText = FullText.Left(37) + TEXT("...");
	}

	DialogueText = FText::FromString(FullText);

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
			.BorderBackgroundColor(this, &SGraphNode_YarnDialogueLine::GetBackgroundColor)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10)
				[
					SNew(STextBlock)
					.Text(DialogueText)
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
					EventText.EqualTo(FText::FromString("None"))
						? SNullWidget::NullWidget
						: SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ColorPicker.RoundedSolidBackground"))
						.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
						.Padding(5)
						[
							SNew(STextBlock)
							.Text(EventText)
							.Justification(ETextJustify::Center)
							.ShadowOffset(FVector2D(1, 1))
							.ShadowColorAndOpacity(FLinearColor::Black)

						]
				]
			]
		]
	];

	CreatePinWidgets();
}

void SGraphNode_YarnDialogueLine::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

FSlateColor SGraphNode_YarnDialogueLine::GetBackgroundColor() const
{
	return GraphNode->GetNodeBodyTintColor();
}
