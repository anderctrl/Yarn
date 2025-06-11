#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectSaveContext.h"
#include "YarnDialogueAsset.generated.h"

class UYarnDialogueEvent;
class UYarnDialogueCondition;

USTRUCT(BlueprintType)
struct FDialogueLine
{
	GENERATED_BODY()

public:
	FDialogueLine()
	{
		LineID = NAME_None;
		SpeakerID = NAME_None;
		Text = FText::FromString("Text");
		JumpToBranchID = NAME_None;
	}

	FDialogueLine(FName InLineID, FName InSpeakerID = NAME_None, const FText& InText = FText::FromString("Text"),
	              FName InJumpToBranchID = NAME_None)
	{
		LineID = InLineID;
		SpeakerID = InSpeakerID;
		Text = InText;
		JumpToBranchID = InJumpToBranchID;
	}

	UPROPERTY(EditAnywhere)
	FName LineID;

	UPROPERTY(EditAnywhere)
	FName SpeakerID;

	UPROPERTY(EditAnywhere, meta=(MultiLine="true", DisplayName="Dialogue Text", LocalizedText))
	FText Text;

	UPROPERTY(EditAnywhere)
	TArray<FName> PlayerChoiceIDs;

	UPROPERTY(EditAnywhere)
	FName JumpToBranchID;

	UPROPERTY(Instanced, EditAnywhere)
	UYarnDialogueEvent* Event = nullptr;

	UPROPERTY(Instanced, EditAnywhere)
	USoundBase* Audio = nullptr;

	bool operator==(const FDialogueLine& Other) const
	{
		return LineID.IsEqual(Other.LineID);
	}
};


USTRUCT(BlueprintType)
struct FDialogueBranch
{
	GENERATED_BODY()

public:
	FDialogueBranch()
	{
		BranchID = NAME_None;
		DisplayName = "Branch";
	}

	FDialogueBranch(FName InBranchID, FString InDisplayName)
	{
		BranchID = InBranchID;
		DisplayName = InDisplayName;
	}

	UPROPERTY(VisibleAnywhere)
	FName BranchID;

	UPROPERTY(EditAnywhere)
	FString DisplayName;

	UPROPERTY(EditAnywhere)
	TArray<FDialogueLine> Lines;

	UPROPERTY(EditAnywhere)
	TArray<FDialogueLine> PlayerChoices;

	UPROPERTY(EditAnywhere, Instanced)
	TArray<UYarnDialogueCondition*> EnterConditions;
};


UCLASS(BlueprintType)
class YARN_API UYarnDialogueAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FDialogueBranch> Branches;
	
	static FName GetMainBranchID()
	{
		return FName("Branch_Main");
	}

	UYarnDialogueAsset()
	{
		CreateMainBranch();
	}

	FDialogueBranch* GetBranchByID(FName ID)
	{
		for (FDialogueBranch& Branch : Branches)
		{
			if (Branch.BranchID == ID)
				return &Branch;
		}
		return nullptr;
	}

	FDialogueLine* GetLineByID(FName ID)
	{
		for (FDialogueBranch& Branch : Branches)
		{
			for (FDialogueLine& Line : Branch.Lines)
			{
				if (Line.LineID == ID)
				{
					return &Line;
				}
			}
		}
		return nullptr;
	}

	FDialogueLine* GetChoiceByID(FName ID)
	{
		for (FDialogueBranch& Branch : Branches)
		{
			for (FDialogueLine& Choice : Branch.PlayerChoices)
			{
				if (Choice.LineID == ID)
				{
					return &Choice;
				}
			}
		}
		return nullptr;
	}

	TArray<FDialogueLine> GetChoicesForLineID(FName ID)
	{
		for (FDialogueBranch& Branch : Branches)
		{
			FDialogueLine* Line = nullptr;

			for (FDialogueLine& L : Branch.Lines)
			{
				if (L.LineID == ID)
				{
					Line = &L;
					break;
				}
			}

			if (Line)
			{
				TArray<FDialogueLine> Result;

				for (const FName& ChoiceID : Line->PlayerChoiceIDs)
				{
					for (const FDialogueLine& Choice : Branch.PlayerChoices)
					{
						if (Choice.LineID == ChoiceID)
						{
							Result.Add(Choice);
							break;
						}
					}
				}

				return Result;
			}
		}

		return {};
	}

	bool GetNextLineInfo(const FName CurrentBranchID, const FName CurrentLineID, FName& OutNextBranchID, FName& OutNextLineID)
	{
		FDialogueBranch* Branch = GetBranchByID(CurrentBranchID);
		if (!Branch)
		{
			return false;
		}

		for (int32 i = 0; i < Branch->Lines.Num(); ++i)
		{
			if (Branch->Lines[i].LineID == CurrentLineID)
			{
				if (i + 1 < Branch->Lines.Num())
				{
					OutNextBranchID = CurrentBranchID;
					OutNextLineID = Branch->Lines[i + 1].LineID;
					return true;
				}

				if (Branch->Lines[i].JumpToBranchID != NAME_None)
				{
					FDialogueBranch* NextBranch = GetBranchByID(Branch->Lines[i].JumpToBranchID);
					if (NextBranch && NextBranch->Lines.Num() > 0)
					{
						OutNextBranchID = NextBranch->BranchID;
						OutNextLineID = NextBranch->Lines[0].LineID;
						return true;
					}
				}
				break;
			}
		}

		return false;
	}


protected:
#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override
	{
		CreateMainBranch();
		Super::PreSave(SaveContext);
	}
#endif

private:
	void CreateMainBranch()
	{
		if (GetBranchByID(GetMainBranchID()) != nullptr)
		{
			return;
		}

		const FString MainLineString = FString::Printf(TEXT("%s_Line_0"), *GetMainBranchID().ToString());

		FDialogueBranch MainBranch = FDialogueBranch(GetMainBranchID(), "Main");
		MainBranch.Lines.Add(FDialogueLine(FName(MainLineString)));
		Branches.Insert(MainBranch, 0);
	}
};