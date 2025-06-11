#pragma once

#include "CoreMinimal.h"

class UYarnDialogueCondition;
class UYarnDialogueEvent;
struct FDialogueLine;
struct FDialogueBranch;
class UYarnDialogueAsset;

class YARNEDITOR_API FYarnDialogueEditorHelpers
{
public:
	static bool TestYarnAssetValidity(UYarnDialogueAsset* Asset);

private:
	static void BuildValidBranchesMap(const UYarnDialogueAsset* Asset, TMap<FName, FString>& OutValidBranches);

	static bool CheckBranchConditions(FDialogueBranch& Branch);

	static bool CheckLines(const FDialogueBranch& Branch, const TMap<FName, FString>& ValidBranches);

	static bool CheckSingleLine(const FDialogueBranch& Branch, const FDialogueLine& Line, const TMap<FName, FString>& ValidBranches);

	static bool IsJumpToBranchIDValid(const FName& JumpToBranchID, const TMap<FName, FString>& ValidBranches);
	static bool IsAudioValid(USoundBase* Sound);
	static bool IsEventValid(UYarnDialogueEvent* Event);
	static bool AreEnterConditionsValid(const TArray<UYarnDialogueCondition*>& EnterConditions);
};
