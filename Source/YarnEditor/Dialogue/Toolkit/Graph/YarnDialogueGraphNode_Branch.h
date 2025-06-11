#pragma once

#include "CoreMinimal.h"
#include "YarnDialogueGraphNode_Base.h"
#include "EdGraph/EdGraphNode.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"
#include "YarnDialogueGraphNode_Branch.generated.h"


UCLASS()
class YARNEDITOR_API UYarnDialogueGraphNode_Branch : public UYarnDialogueGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FName BranchID;

	UPROPERTY()
	UYarnDialogueAsset* DialogueAsset;

	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	UEdGraphPin* GetOutputPin() const;
	UEdGraphPin* GetInputPin() const;
	virtual FLinearColor GetNodeBodyTintColor() const override;
};
