#pragma once

#include "CoreMinimal.h"
#include "YarnDialogueGraphNode_Base.h"
#include "EdGraph/EdGraphNode.h"
#include "YarnDialogueGraphNode_Line.generated.h"

class UYarnDialogueAsset;

UCLASS()
class YARNEDITOR_API UYarnDialogueGraphNode_Line : public UYarnDialogueGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FName LineID;

	UPROPERTY()
	FName ParentBranchID;

	UPROPERTY()
	UYarnDialogueAsset* DialogueAsset;

	virtual void AllocateDefaultPins() override;

	UEdGraphPin* GetOutputPin() const;
	UEdGraphPin* GetInputPin() const;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeBodyTintColor() const override;
};
