#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YarnSettings.generated.h"

class AYarnDialogueCameraActor;
class UDialogueSpeakerList;
/**
 * 
 */
UCLASS(Config = Engine, DefaultConfig, meta = (DisplayName = "Yarn"))
class YARN_API UYarnSettings : public UObject
{
	GENERATED_BODY()

public:
	UYarnSettings(const FObjectInitializer& ObjectInitializer);

	static const UYarnSettings* Get();

	UDialogueSpeakerList* GetLoadedSpeakerList() const;

	UPROPERTY(Config, EditAnywhere, Category = "Dialogue",
		meta = (AllowedClasses = "/Script/Yarn.DialogueSpeakerList", DisplayThumbnail = true))
	TSoftObjectPtr<UDialogueSpeakerList> GlobalSpeakerList;

	UPROPERTY(Config, EditAnywhere, Category = "Dialogue|Camera",
		meta = (AllowedClasses = "/Script/Yarn.YarnDialogueCameraActor", DisplayThumbnail = true))
	TSoftClassPtr<AYarnDialogueCameraActor> DialogueCameraClass;

	UPROPERTY(Config, EditAnywhere, Category = "Dialogue|Timing")
	float BaseWordsPerMinute = 160.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Dialogue|Timing")
	float MinimumDialogueDisplayTime = 1.f;
};
