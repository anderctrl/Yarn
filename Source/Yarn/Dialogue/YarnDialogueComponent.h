#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "YarnDialogueComponent.generated.h"

class UDialogueWidget;
class UYarnDialogueAsset;
class AYarnDialogueCameraActor;

USTRUCT(BlueprintType)
struct FDialogueChoiceInfo
{
    GENERATED_BODY()

    FDialogueChoiceInfo()
    {
       LineID = NAME_None;
       Text = FText::GetEmpty();
    }

    FDialogueChoiceInfo(const FName& InLineID, const FText& InText)
    {
       LineID = InLineID;
       Text = InText;
    }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName LineID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FText Text;
};

USTRUCT(BlueprintType)
struct FCurrentDialogueState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName BranchID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName LineID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName SpeakerID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FText SpeakerName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FText LineText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USoundBase* LineAudio = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<FDialogueChoiceInfo> Choices;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStateUpdated, const FCurrentDialogueState&, State, const float, TimeForNext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YARN_API UYarnDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UYarnDialogueComponent();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(UYarnDialogueAsset* InDialogueAsset);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayNextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SpectatePlayerDialogue(APawn* PawnToSpectate);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopSpectatingPlayerDialogue();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerStartDialogue(UYarnDialogueAsset* InDialogueAsset);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerPlayDialogueLine(FName BranchID, FName LineID);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerOptionChosen(FName ChoiceID);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastStopDialogue();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayAudio(USoundBase* SoundToPlay);

    UPROPERTY(BlueprintAssignable)
    FOnDialogueStateUpdated OnDialogueStateUpdated;

    UPROPERTY(BlueprintAssignable)
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UDialogueWidget> DialogueWidgetClass;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    UFUNCTION()
    void OnRep_CurrentDialogueState();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UAudioComponent* YarnAudioComponent;
    
    UPROPERTY(ReplicatedUsing=OnRep_CurrentDialogueState)
    FCurrentDialogueState CurrentDialogueState;

private:
    UPROPERTY(Replicated)
    UYarnDialogueAsset* CurrentDialogueAsset = nullptr;
    
    UPROPERTY(Replicated)
    AYarnDialogueCameraActor* DialogueCam;
    
    UPROPERTY(Replicated)
    APawn* SpectatingPlayer = nullptr;

    UPROPERTY()
    UDialogueWidget* DialogueWidgetInstance;

    FTimerHandle AutoPlayNextLineTimerHandle;
    FTimerHandle CameraShiftToPlayerTimerHandle;

    void HandleStartDialogue(UYarnDialogueAsset* InDialogueAsset);
    void HandlePlayDialogueLine(FName BranchID, FName LineID);
    void HandleOptionChosen(FName ChoiceID);
    void HandleStopDialogue();

    bool ValidateDialogueAsset(UYarnDialogueAsset* InDialogueAsset) const;
    void UpdateDialogueCam(const FName DialogueLineID);

    UFUNCTION()
    void OnOptionChosen(const FName ChoiceID);

    static float CalculateDialogueDisplayTime(const FString& DialogueLineText);
};