#include "SDialogueLogWindow.h"

#include "YarnEditorModule.h"
#include "Dialogue/Toolkit/DialogueRedirectLog.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/AppStyle.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Text/SRichTextBlock.h"

void SDialogueLogWindow::Construct(const FArguments& InArgs)
{
	const FSlateFontInfo LogFont = FCoreStyle::Get().GetFontStyle("Log.Font");

	InfoStyle = FTextBlockStyle()
	            .SetFont(LogFont)
	            .SetColorAndOpacity(FSlateColor(FLinearColor::White));

	FTextBlockStyle WarningStyle = InfoStyle;
	WarningStyle.SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));

	FTextBlockStyle ErrorStyle = InfoStyle;
	ErrorStyle.SetColorAndOpacity(FSlateColor(FLinearColor::Red));

	FTextBlockStyle TransparentTextStyle;
	TransparentTextStyle
		.SetFont(LogFont)
		.SetColorAndOpacity(FLinearColor(0, 0, 0, 0))
		.SetSelectedBackgroundColor(FLinearColor(0.2f, 0.4f, 0.7f, 0.5f));

	LogTextBuffer = TEXT("");

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(6)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FSlateColor(EStyleColor::Background))
			[
				SAssignNew(ScrollBox, SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SOverlay)
					+ SOverlay::Slot() // Invisible, selectable
					.Padding(4, 2)
					[
						SAssignNew(MultiLineText, SMultiLineEditableText)
						.Text(FText::FromString(LogTextBuffer))
						.IsReadOnly(true)
						.AutoWrapText(true)
						.TextStyle(&TransparentTextStyle)
					]
					+ SOverlay::Slot() // Visible
					.Padding(4, 2)
					[
						SAssignNew(LogLinesBox, SVerticalBox)
						.Visibility(EVisibility::HitTestInvisible)
					]
				]
			]
		]
	];

	LogDelegateHandle = FDialogueRedirectLog::Get().OnLogMessageReceived.AddSP(
		SharedThis(this), &SDialogueLogWindow::OnNewLogReceived);
}

void SDialogueLogWindow::OnNewLogReceived(const FString& LogLine, ELogVerbosity::Type Verbosity, FName Category)
{
	FName Style;

	switch (Verbosity)
	{
	case ELogVerbosity::Error:
		Style = FName("Log.Error");
		break;
	case ELogVerbosity::Warning:
		Style = FName("Log.Warning");
		break;
	case ELogVerbosity::Display:
		Style = FName("Log.Success");
		break;
	default:
		Style = FName("Log.Info");
		break;
	}

	LogLinesBox->AddSlot()
	           .AutoHeight()
	[
		SNew(SRichTextBlock)
		.Text(FText::FromString(LogLine))
		.TextStyle(FYarnEditorModule::GetStyleSet(), Style)
		.DecoratorStyleSet(&FCoreStyle::Get())
	];

	LogTextBuffer += LogLine + TEXT("\n");
	MultiLineText->SetText(FText::FromString(LogTextBuffer));

	ScrollBox->ScrollToEnd();
}

SDialogueLogWindow::~SDialogueLogWindow()
{
	if (LogDelegateHandle.IsValid())
	{
		FDialogueRedirectLog::Get().OnLogMessageReceived.Remove(LogDelegateHandle);
	}
}
