#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMultiLineEditableText;
class SScrollBox;

class SDialogueLogWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogueLogWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void OnNewLogReceived(const FString& LogLine, ELogVerbosity::Type Verbosity, FName Category);

	virtual ~SDialogueLogWindow() override;

private:
	TSharedPtr<SScrollBox> ScrollBox;
	TSharedPtr<SVerticalBox> LogLinesBox;
	TArray<FString> LogLines;
	TSharedPtr<SMultiLineEditableText> MultiLineText;
	
	FDelegateHandle LogDelegateHandle;

	FTextBlockStyle InfoStyle;

	FString LogTextBuffer;
};
