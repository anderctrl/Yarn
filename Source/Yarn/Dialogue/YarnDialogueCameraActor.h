#pragma once

#include "CoreMinimal.h"
#include "CineCameraActor.h"
#include "CineCameraSettings.h"
#include "GameFramework/Actor.h"
#include "YarnDialogueCameraActor.generated.h"

UCLASS()
class YARN_API AYarnDialogueCameraActor : public ACineCameraActor
{
	GENERATED_BODY()

public:
	AYarnDialogueCameraActor(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void MoveToCenterOfActors(UPARAM(ref)
	const TArray<AActor*>& Actors);

	UFUNCTION(BlueprintCallable)
	void MoveCameraForDialogue(AActor* SpeakerActor, AActor* ListenerActor);

	UFUNCTION(BlueprintCallable)
	void FocusOnActor(AActor* InActor);

	UFUNCTION(BlueprintCallable)
	void FocusOnActorByTag(FName ActorTag);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_FocusSettings)
	FCameraFocusSettings ReplicatedFocusSettings;

	UPROPERTY(ReplicatedUsing=OnRep_FieldOfView)
	float ReplicatedFieldOfView;

	UPROPERTY(ReplicatedUsing=OnRep_LookAtTrackingSettings)
	FCameraLookatTrackingSettings ReplicatedLookatTrackingSettings;

	UFUNCTION()
	void OnRep_FocusSettings();

	UFUNCTION()
	void OnRep_FieldOfView();

	UFUNCTION()
	void OnRep_LookAtTrackingSettings();

private:
	static FVector GetHeadLocation(const AActor* Actor);
};