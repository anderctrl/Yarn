#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "DialogueChoiceButton.generated.h"

class UCommonTextBlock;
/**
 * 
 */
UCLASS()
class YARN_API UDialogueChoiceButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	void SetText(const FText& NewText) const;

protected:
	virtual void NativeConstruct() override;
	virtual void HandleFocusReceived() override;
	virtual void HandleFocusLost() override;

	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;

	
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UCommonTextBlock* TextBlock;
};
