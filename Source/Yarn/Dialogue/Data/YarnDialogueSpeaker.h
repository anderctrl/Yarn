#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectSaveContext.h"
#include "YarnDialogueSpeaker.generated.h"

class UYarnDialogueCondition;

USTRUCT(BlueprintType)
struct FDialogueSpeaker {
	GENERATED_BODY()

public:
	FDialogueSpeaker()
	{
		SpeakerID = NAME_None;
		DisplayName = FText::FromString("Speaker");
	}
	FDialogueSpeaker(FName InSpeakerID, const FText& InDisplayName)
	{
		SpeakerID = InSpeakerID;
		DisplayName = InDisplayName;
	}
	
	UPROPERTY(EditAnywhere)
	FName SpeakerID;

	UPROPERTY(EditAnywhere)
	FText DisplayName;
};

UCLASS(BlueprintType)
class YARN_API UDialogueSpeakerList : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UDialogueSpeakerList()
	{
		CreatePlayerSpeaker();
	}
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDialogueSpeaker> Speakers;

	const FDialogueSpeaker* FindSpeakerByID(FName InID) const
	{
		for (const FDialogueSpeaker& Speaker : Speakers)
		{
			if (Speaker.SpeakerID == InID)
			{
				return &Speaker;
			}
		}
		return nullptr;
	}
protected:
#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override
	{
		CreatePlayerSpeaker();
		Super::PreSave(SaveContext);
	}
#endif
private:
	void CreatePlayerSpeaker()
	{
		if (FindSpeakerByID("Player") != nullptr)
		{
			return;
		}
		Speakers.Insert(FDialogueSpeaker("Player", FText::FromString("Player")), 0);
	}
};