#include "YarnDialogueEditorHelpers.h"

#include "YarnEditorModule.h"
#include "Yarn/YarnSettings.h"
#include "Yarn/Dialogue/Data/YarnDialogueAsset.h"
#include "Yarn/Dialogue/Data/YarnDialogueCondition.h"
#include "Yarn/Dialogue/Data/YarnDialogueEvent.h"
#include "Yarn/Dialogue/Data/YarnDialogueSpeaker.h"

bool FYarnDialogueEditorHelpers::TestYarnAssetValidity(UYarnDialogueAsset* Asset)
{
	if (!Asset)
	{
		UE_LOG(LogYarnDialogueEditor, Error, TEXT("Asset is null"));
		return false;
	}

	UDialogueSpeakerList* SpeakerList = UYarnSettings::Get()->GetLoadedSpeakerList();

	if (SpeakerList)
	{
		if (SpeakerList->Speakers.IsEmpty())
		{
			UE_LOG(LogYarnDialogueEditor, Warning, TEXT("Speaker list is empty!"));
		}
	}
	else
	{
		UE_LOG(LogYarnDialogueEditor, Warning, TEXT("No speaker list provided, add it under the plugin settings!"));
	}

	TMap<FName, FString> ValidBranches;
	BuildValidBranchesMap(Asset, ValidBranches);

	for (FDialogueBranch& Branch : Asset->Branches)
	{
		if (!CheckBranchConditions(Branch))
		{
			return false;
		}

		if (!AreEnterConditionsValid(Branch.EnterConditions))
		{
			UE_LOG(LogYarnDialogueEditor, Error, TEXT("Invalid enter conditions in branch '%s'"), *Branch.BranchID.ToString());
			return false;
		}

		if (!CheckLines(Branch, ValidBranches))
		{
			return false;
		}
	}

	UE_LOG(LogYarnDialogueEditor, Display, TEXT("Good to go."));
	return true;
}

void FYarnDialogueEditorHelpers::BuildValidBranchesMap(const UYarnDialogueAsset* Asset, TMap<FName, FString>& OutValidBranches)
{
	for (const FDialogueBranch& Branch : Asset->Branches)
	{
		OutValidBranches.Add(Branch.BranchID, Branch.DisplayName);
	}
}

bool FYarnDialogueEditorHelpers::CheckBranchConditions(FDialogueBranch& Branch)
{
	if (Branch.BranchID == FName(UYarnDialogueAsset::GetMainBranchID()))
	{
		if (Branch.EnterConditions.Num() > 0)
		{
			Branch.EnterConditions.Empty();
			UE_LOG(LogYarnDialogueEditor, Warning, TEXT("Main branch had enter conditions, cleared conditions."));
		}
	}
	return true;
}

bool FYarnDialogueEditorHelpers::CheckLines(const FDialogueBranch& Branch, const TMap<FName, FString>& ValidBranches)
{
	for (const FDialogueLine& Line : Branch.Lines)
	{
		if (!CheckSingleLine(Branch, Line, ValidBranches))
		{
			return false;
		}
	}
	return true;
}

bool FYarnDialogueEditorHelpers::CheckSingleLine(const FDialogueBranch& Branch, const FDialogueLine& Line, const TMap<FName, FString>& ValidBranches)
{
    if (!IsJumpToBranchIDValid(Line.JumpToBranchID, ValidBranches))
    {
        UE_LOG(LogYarnDialogueEditor, Error,
            TEXT("Invalid JumpToBranchID '%s' in line '%s' of branch '%s'"),
            *Line.JumpToBranchID.ToString(), *Line.LineID.ToString(), *Branch.BranchID.ToString());
        return false;
    }

    if (!IsEventValid(Line.Event))
    {
        UE_LOG(LogYarnDialogueEditor, Error,
            TEXT("Invalid Event on line '%s' of branch '%s'"),
            *Line.LineID.ToString(), *Branch.BranchID.ToString());
        return false;
    }

    if (!IsAudioValid(Line.Audio))
    {
        UE_LOG(LogYarnDialogueEditor, Error,
            TEXT("Invalid Audio on line '%s' of branch '%s'"),
            *Line.LineID.ToString(), *Branch.BranchID.ToString());
        return false;
    }

    for (const FName& ChoiceID : Line.PlayerChoiceIDs)
    {
        const FDialogueLine* Choice = Branch.PlayerChoices.FindByPredicate([&ChoiceID](const FDialogueLine& C) {
            return C.LineID == ChoiceID;
        });

        if (!Choice)
        {
            UE_LOG(LogYarnDialogueEditor, Error,
                TEXT("Choice ID '%s' referenced in line '%s' not found in PlayerChoices of branch '%s'"),
                *ChoiceID.ToString(), *Line.LineID.ToString(), *Branch.BranchID.ToString());
            return false;
        }

        if (!IsJumpToBranchIDValid(Choice->JumpToBranchID, ValidBranches))
        {
            UE_LOG(LogYarnDialogueEditor, Error,
                TEXT("Invalid JumpToBranchID '%s' in choice '%s' linked to line '%s' of branch '%s'"),
                *Choice->JumpToBranchID.ToString(), *Choice->LineID.ToString(), *Line.LineID.ToString(), *Branch.BranchID.ToString());
            return false;
        }

        if (!IsEventValid(Choice->Event))
        {
            UE_LOG(LogYarnDialogueEditor, Error,
                TEXT("Invalid Event on choice '%s' linked to line '%s' of branch '%s'"),
                *Choice->LineID.ToString(), *Line.LineID.ToString(), *Branch.BranchID.ToString());
            return false;
        }

        if (!IsAudioValid(Choice->Audio))
        {
            UE_LOG(LogYarnDialogueEditor, Error,
                TEXT("Invalid Audio on choice '%s' linked to line '%s' of branch '%s'"),
                *Choice->LineID.ToString(), *Line.LineID.ToString(), *Branch.BranchID.ToString());
            return false;
        }
    }

    return true;
}

bool FYarnDialogueEditorHelpers::IsJumpToBranchIDValid(const FName& JumpToBranchID, const TMap<FName, FString>& ValidBranches)
{
	return (JumpToBranchID == NAME_None) || ValidBranches.Contains(JumpToBranchID);
}

bool FYarnDialogueEditorHelpers::IsAudioValid(USoundBase* Sound)
{
	return (Sound == nullptr) || Sound->IsValidLowLevelFast();
}

bool FYarnDialogueEditorHelpers::IsEventValid(UYarnDialogueEvent* Event)
{
	return (Event == nullptr) || Event->IsValidLowLevelFast();
}

bool FYarnDialogueEditorHelpers::AreEnterConditionsValid(const TArray<UYarnDialogueCondition*>& EnterConditions)
{
	for (const UYarnDialogueCondition* Condition : EnterConditions)
	{
		if (!Condition || !Condition->IsValidLowLevelFast())
		{
			UE_LOG(LogYarnDialogueEditor, Error, TEXT("Found invalid enter condition."));
			return false;
		}
	}
	return true;
}
