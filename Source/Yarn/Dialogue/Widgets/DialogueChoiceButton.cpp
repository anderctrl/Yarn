#include "DialogueChoiceButton.h"

#include "CommonTextBlock.h"

void UDialogueChoiceButton::SetText(const FText& NewText) const
{
	if (TextBlock)
	{
		TextBlock->SetText(NewText);
	}
}

void UDialogueChoiceButton::NativeConstruct()
{
	Super::NativeConstruct();
	TextBlock->SetStyle(GetStyle()->NormalTextStyle);
}

void UDialogueChoiceButton::HandleFocusReceived()
{
	Super::HandleFocusReceived();
	TextBlock->SetStyle(GetStyle()->NormalHoveredTextStyle);
}

void UDialogueChoiceButton::HandleFocusLost()
{
	Super::HandleFocusLost();
	TextBlock->SetStyle(GetStyle()->NormalTextStyle);
}

void UDialogueChoiceButton::NativeOnHovered()
{
	Super::NativeOnHovered();
	SetFocus();
}

void UDialogueChoiceButton::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
	TextBlock->SetStyle(GetStyle()->NormalTextStyle);
}

FReply UDialogueChoiceButton::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Reply = Super::NativeOnKeyUp(InGeometry, InKeyEvent);
	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept
		|| InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		if ( PressMethod == EButtonPressMethod::ButtonRelease || ( PressMethod == EButtonPressMethod::DownAndUp) )
		{
			OnClicked().Broadcast();
		}
	}
	return Reply;
}

FReply UDialogueChoiceButton::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Reply = Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept
		|| InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		if ( PressMethod == EButtonPressMethod::ButtonPress || ( PressMethod == EButtonPressMethod::DownAndUp) )
		{
			OnPressed().Broadcast();
		}
	}
	return Reply;
}
