#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"
#include "YarnDialogueGraph.generated.h"

class UYarnDialogueGraphNode;
class UYarnDialogueGraphNode_Branch;
class UYarnDialogueGraphNode_Line;

/**
 * Graph representation of a Yarn Dialogue Asset
 */
UCLASS()
class YARNEDITOR_API UYarnDialogueGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	void Initialize(UYarnDialogueAsset* InDialogueAsset);
	void CreateBranches(int32 BranchVerticalSpacing);
	void CreateLinesAndChoices(int32 BaseLineHorizontalSpacing, int32 ChoiceHorizontalOffset,
	                           int32 ChoiceVerticalSpacing,
	                           float BaseNodeWidth, float WidthPerCharacter, float MaxNodeWidthForSpacing);
	void ConnectChoicesToNextMainLine();
	void ConnectJumpToBranches();
	void CenterNodesVertically(float NodeHeight);

	UPROPERTY()
	UYarnDialogueAsset* DialogueAsset;

private:
	UPROPERTY()
	TMap<FName, UYarnDialogueGraphNode_Branch*> BranchIDToNode;
	
	UPROPERTY()
	TMap<FName, UYarnDialogueGraphNode_Line*> LineIDToNode;
};