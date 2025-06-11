#include "DialogueWidget.h"
#include "CommonTextBlock.h"
#include "DialogueChoiceButton.h"
#include "Components/VerticalBox.h"

#include "Dialogue/YarnDialogueComponent.h"

void UDialogueWidget::OnDialogueStateUpdated_Implementation(const FCurrentDialogueState& State, const float TimeForNext)
{
    LastLineText->SetText(LastLineTextCache);

    ChoicesBox->ClearChildren();

    FText TextToDisplay = State.LineText;
    if (SpeakerText)
    {
       SpeakerText->SetText(State.SpeakerName);
    }
    else
    {
       TextToDisplay = FText::Format(
          NSLOCTEXT("Dialogue", "SpokenLine", "{0}: {1}"),
          State.SpeakerName,
          State.LineText
       );
    }
    DialogueText->SetText(TextToDisplay);

    LastLineTextCache = TextToDisplay;

    if (State.Choices.IsEmpty())
    {
       return;
    }

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&, State]()
    {
       for (const auto& Choice : State.Choices)
       {
          UDialogueChoiceButton* NewButton = CreateWidget<UDialogueChoiceButton>(GetWorld(), ButtonWidgetClass);
          NewButton->SetText(Choice.Text);

          if (bIsSpectating)
          {
             NewButton->DisableButtonWithReason(NSLOCTEXT("Dialogue", "SpectatorCantChoose", "Can't manage this dialogue"));
          }
          else
          {
             NewButton->OnPressed().AddWeakLambda(this, [this, Choice]()
             {
                LastLineTextCache = FText::Format(
                   NSLOCTEXT("Dialogue", "PlayerSpokenLine", "Player: {0}"),
                   Choice.Text
                );
                OnOptionChosen.Broadcast(Choice.LineID);
             });
          }

          ChoicesBox->AddChild(NewButton);
       }
       if (ChoicesBox->HasAnyChildren())
       {
          ChoicesBox->GetChildAt(0)->SetFocus();
       }
    }, TimeForNext, false);
}


void UDialogueWidget::InitDialogueWidget(UYarnDialogueComponent* NewDialogueComponent, const bool bInIsSpectating)
{
    if (IsValid(NewDialogueComponent))
    {
       DialogueComponent = NewDialogueComponent;
    }

    bIsSpectating = bInIsSpectating;

    if (NewDialogueComponent == nullptr)
    {
       return;
    }

    DialogueComponent->OnDialogueStateUpdated.AddDynamic(this, &UDialogueWidget::OnDialogueStateUpdated);
}

void UDialogueWidget::UpdateDialogueWidget(const FCurrentDialogueState& State, const float TimeForNext)
{
    OnDialogueStateUpdated_Implementation(State, TimeForNext);
}

void UDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (GetOwningPlayer())
    {
       GetOwningPlayer()->SetInputMode(FInputModeUIOnly());
       GetOwningPlayer()->SetShowMouseCursor(true);
    }
}

void UDialogueWidget::NativeDestruct()
{
    Super::NativeDestruct();

    if (GetOwningPlayer())
    {
       GetOwningPlayer()->SetInputMode(FInputModeGameOnly());
       GetOwningPlayer()->SetShowMouseCursor(false);
    }
}