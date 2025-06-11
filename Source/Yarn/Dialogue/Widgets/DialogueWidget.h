#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DialogueWidget.generated.h"

class UDialogueChoiceButton;
class UYarnDialogueComponent;
struct FCurrentDialogueState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionChosen, FName, ChoiceID);

UCLASS()
class YARN_API UDialogueWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitDialogueWidget(UYarnDialogueComponent* NewDialogueComponent, bool bInIsSpectating);

	UFUNCTION(BlueprintCallable)
	void UpdateDialogueWidget(const FCurrentDialogueState& State, float TimeForNext);

	UPROPERTY(BlueprintAssignable)
	FOnOptionChosen OnOptionChosen;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue")
	void OnDialogueStateUpdated(const FCurrentDialogueState& State, const float TimeForNext);

	UPROPERTY(EditAnywhere, meta = (BindWidget, OptionalWidget=true))
	class UCommonTextBlock* SpeakerText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UCommonTextBlock* DialogueText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UCommonTextBlock* LastLineText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UVerticalBox* ChoicesBox;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDialogueChoiceButton> ButtonWidgetClass;

	UPROPERTY()
	bool bIsSpectating = false;

	UPROPERTY()
	float WaitTime;

	UPROPERTY()
	UYarnDialogueComponent* DialogueComponent;

private:
	UPROPERTY()
	FText LastLineTextCache;
};
