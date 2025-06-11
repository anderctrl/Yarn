#include "YarnSettings.h"

#include "YarnModule.h"
#include "Dialogue/Data/YarnDialogueSpeaker.h"

UYarnSettings::UYarnSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UYarnSettings* UYarnSettings::Get()
{
	return GetDefault<UYarnSettings>();
}

UDialogueSpeakerList* UYarnSettings::GetLoadedSpeakerList() const
{
	if (GlobalSpeakerList.IsNull())
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("GlobalSpeakerList is null (not set in settings)"));
		return nullptr;
	}

	UObject* LoadedObj = GlobalSpeakerList.LoadSynchronous();

	if (!LoadedObj)
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("Failed to load GlobalSpeakerList asset"));
		return nullptr;
	}

	UDialogueSpeakerList* SpeakerList = Cast<UDialogueSpeakerList>(LoadedObj);
	if (!SpeakerList)
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("Loaded asset is not of type UDialogueSpeakerList"));
		return nullptr;
	}

	return SpeakerList;
}