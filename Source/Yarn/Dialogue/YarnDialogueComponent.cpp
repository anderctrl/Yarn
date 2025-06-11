#include "YarnDialogueComponent.h"

#include "EngineUtils.h"
#include "Data/YarnDialogueAsset.h"
#include "YarnDialogueCameraActor.h"
#include "YarnModule.h"
#include "YarnSettings.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "Data/YarnDialogueSpeaker.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/DialogueWidget.h"

UYarnDialogueComponent::UYarnDialogueComponent()
{
	SetIsReplicatedByDefault(true);

	YarnAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("YarnAudioComponent"));
	YarnAudioComponent->SetIsReplicated(true);
	YarnAudioComponent->SetAutoActivate(false);
}

void UYarnDialogueComponent::MulticastPlayAudio_Implementation(USoundBase* SoundToPlay)
{
	YarnAudioComponent->SetSound(SoundToPlay);
	YarnAudioComponent->Play();
}

void UYarnDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentDialogueState = FCurrentDialogueState();

	if (USceneComponent* RootComp = GetOwner()->GetRootComponent())
	{
		YarnAudioComponent->AttachToComponent(RootComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		YarnAudioComponent->RegisterComponent();
	}
}

void UYarnDialogueComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UYarnDialogueComponent, CurrentDialogueState);
	DOREPLIFETIME(UYarnDialogueComponent, CurrentDialogueAsset);
	DOREPLIFETIME(UYarnDialogueComponent, DialogueCam);
}


void UYarnDialogueComponent::OnRep_CurrentDialogueState()
{
	if (CurrentDialogueState.LineID.IsNone() && CurrentDialogueState.BranchID.IsNone())
	{
		return;
	}

	OnDialogueStateUpdated.Broadcast(CurrentDialogueState,
	                                 CalculateDialogueDisplayTime(CurrentDialogueState.LineText.ToString()));
}

void UYarnDialogueComponent::StartDialogue(UYarnDialogueAsset* InDialogueAsset)
{
	if (!InDialogueAsset)
		return;

	StopDialogue();

	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			if (DialogueWidgetInstance)
			{
				DialogueWidgetInstance->RemoveFromParent();
				DialogueWidgetInstance = nullptr;
			}

			DialogueWidgetInstance = CreateWidget<UDialogueWidget>(PC, DialogueWidgetClass);
			DialogueWidgetInstance->InitDialogueWidget(this, false);
			DialogueWidgetInstance->OnOptionChosen.AddDynamic(this, &UYarnDialogueComponent::OnOptionChosen);
			DialogueWidgetInstance->AddToViewport();
		}
	}

	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerStartDialogue(InDialogueAsset);
		return;
	}

	HandleStartDialogue(InDialogueAsset);
}

void UYarnDialogueComponent::PlayNextDialogueLine()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoPlayNextLineTimerHandle);

	if (!CurrentDialogueState.Choices.IsEmpty())
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("Cannot play next dialogue line, player has to choose!"));
		return;
	}

	FName NewBranchID;
	FName NewLineID;

	const bool bFoundNextLine = CurrentDialogueAsset->GetNextLineInfo(CurrentDialogueState.BranchID,
	                                                                  CurrentDialogueState.LineID,
	                                                                  NewBranchID, NewLineID);

	if (bFoundNextLine)
	{
		if (GetOwnerRole() < ROLE_Authority)
		{
			ServerPlayDialogueLine(NewBranchID, NewLineID);
			return;
		}

		HandlePlayDialogueLine(NewBranchID, NewLineID);
	}
	else
	{
		StopDialogue();
	}
}

void UYarnDialogueComponent::StopDialogue()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		HandleStopDialogue();
		MulticastStopDialogue();
	}
	else if (GetOwnerRole() < ROLE_Authority)
	{
		HandleStopDialogue();
	}
}

void UYarnDialogueComponent::SpectatePlayerDialogue(APawn* PawnToSpectate)
{
	if (PawnToSpectate == GetOwner() || CurrentDialogueAsset != nullptr)
	{
		return;
	}

	UYarnDialogueComponent* SpectatingDialogueComponent = Cast<UYarnDialogueComponent>(
		PawnToSpectate->GetComponentByClass(StaticClass()));

	if (!SpectatingDialogueComponent)
	{
		UE_LOG(LogYarnRuntime, Error,
		       TEXT(
			       "SpectatePlayerDialogue: Failed to get YarnDialogueComponent from PawnToSpectate: %s (Spectator: %s)"
		       ),
		       *GetNameSafe(PawnToSpectate), *GetNameSafe(GetOwner()));
		return;
	}

	if (SpectatingDialogueComponent->CurrentDialogueState.LineID.IsNone())
	{
		UE_LOG(LogYarnRuntime, Warning,
		       TEXT("Attempted to spectate player %s, but is not currently in dialogue. (Spectator: %s)"),
		       *PawnToSpectate->GetName(), *GetNameSafe(GetOwner()));
		return;
	}
	StopSpectatingPlayerDialogue();

	SpectatingPlayer = PawnToSpectate;

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC || !DialogueWidgetClass)
	{
		return;
	}

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->RemoveFromParent();
		DialogueWidgetInstance = nullptr;
	}

	DialogueWidgetInstance = CreateWidget<UDialogueWidget>(PC, DialogueWidgetClass);
	DialogueWidgetInstance->InitDialogueWidget(SpectatingDialogueComponent, true);
	DialogueWidgetInstance->UpdateDialogueWidget(SpectatingDialogueComponent->CurrentDialogueState,
	                                             CalculateDialogueDisplayTime(
		                                             SpectatingDialogueComponent->CurrentDialogueState.LineText.
		                                             ToString()));
	DialogueWidgetInstance->AddToViewport();


	SpectatingDialogueComponent->OnDialogueEnded.RemoveDynamic(
		this, &UYarnDialogueComponent::StopSpectatingPlayerDialogue);
	SpectatingDialogueComponent->OnDialogueEnded.AddDynamic(
		this, &UYarnDialogueComponent::StopSpectatingPlayerDialogue);

	if (SpectatingDialogueComponent->DialogueCam)
	{
		PC->SetViewTargetWithBlend(SpectatingDialogueComponent->DialogueCam, 0.f);
	}
}

void UYarnDialogueComponent::StopSpectatingPlayerDialogue()
{
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->RemoveFromParent();
		DialogueWidgetInstance = nullptr;
	}

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			PC->SetViewTargetWithBlend(OwnerPawn, 0.f);
		}
		else
		{
			UE_LOG(LogYarnRuntime, Warning, TEXT("Spectator %s has OwnerPawn but no PlayerController to reset camera."),
			       *GetNameSafe(GetOwner()));
		}
	}
	else
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("Spectator component %s has no OwnerPawn to reset camera."),
		       *GetNameSafe(this));
	}

	if (SpectatingPlayer)
	{
		if (UYarnDialogueComponent* SpectatingDialogueComponent = Cast<UYarnDialogueComponent>(
			SpectatingPlayer->GetComponentByClass(StaticClass())))
		{
			if (IsValid(SpectatingDialogueComponent))
			{
				SpectatingDialogueComponent->OnDialogueEnded.RemoveDynamic(
					this, &UYarnDialogueComponent::StopSpectatingPlayerDialogue);
			}
			else
			{
				UE_LOG(LogYarnRuntime, Warning,
				       TEXT("SpectatingDialogueComponent is invalid when trying to unbind for spectator %s."),
				       *GetNameSafe(GetOwner()));
			}
		}
		else
		{
			UE_LOG(LogYarnRuntime, Warning,
			       TEXT("SpectatingPlayer %s had no YarnDialogueComponent when trying to unbind for spectator %s."),
			       *GetNameSafe(SpectatingPlayer), *GetNameSafe(GetOwner()));
		}
	}

	SpectatingPlayer = nullptr;
}

void UYarnDialogueComponent::ServerOptionChosen_Implementation(FName ChoiceID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		HandleOptionChosen(ChoiceID);
	}
}

bool UYarnDialogueComponent::ServerOptionChosen_Validate(FName ChoiceID)
{
	return true;
}

void UYarnDialogueComponent::MulticastStopDialogue_Implementation()
{
	HandleStopDialogue();
}

bool UYarnDialogueComponent::ValidateDialogueAsset(UYarnDialogueAsset* InDialogueAsset) const
{
	if (!InDialogueAsset)
		return false;

	const auto* MainBranch = InDialogueAsset->GetBranchByID(UYarnDialogueAsset::GetMainBranchID());
	if (!MainBranch || MainBranch->Lines.IsEmpty())
		return false;

	return true;
}

void UYarnDialogueComponent::HandleStartDialogue(UYarnDialogueAsset* InDialogueAsset)
{
	if (!ValidateDialogueAsset(InDialogueAsset))
		return;

	CurrentDialogueAsset = InDialogueAsset;

	const FDialogueBranch* MainBranch = CurrentDialogueAsset->GetBranchByID(UYarnDialogueAsset::GetMainBranchID());
	if (!MainBranch || MainBranch->Lines.IsEmpty())
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("Invalid main branch or empty lines in dialogue asset."));
		StopDialogue();
		return;
	}

	if (!DialogueCam && GetOwnerRole() == ROLE_Authority)
	{
		DialogueCam = GetWorld()->SpawnActor<AYarnDialogueCameraActor>(AYarnDialogueCameraActor::StaticClass());
	}

	const FName MainLineID = MainBranch->Lines[0].LineID;

	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			PC->SetViewTargetWithBlend(DialogueCam, 0.f);
		}
	}

	HandlePlayDialogueLine(UYarnDialogueAsset::GetMainBranchID(), MainLineID);
}

void UYarnDialogueComponent::HandlePlayDialogueLine(FName BranchID, FName LineID)
{
	GetWorld()->GetTimerManager().ClearTimer(AutoPlayNextLineTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(CameraShiftToPlayerTimerHandle);

	if (!CurrentDialogueAsset)
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("DialogueAsset is null when trying to play a line."));
		StopDialogue();
		return;
	}

	const FDialogueBranch* Branch = CurrentDialogueAsset->GetBranchByID(BranchID);
	if (!Branch)
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("Branch %s not found in dialogue asset."), *BranchID.ToString());
		StopDialogue();
		return;
	}

	const FDialogueLine* Line = CurrentDialogueAsset->GetLineByID(LineID);
	if (!Line)
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("Line %s not found in dialogue asset."), *LineID.ToString());
		StopDialogue();
		return;
	}

	FCurrentDialogueState NewState;
	NewState.BranchID = BranchID;
	NewState.LineID = LineID;
	NewState.SpeakerID = Line->SpeakerID;
	NewState.SpeakerName = FText::FromString("Unknown");
	NewState.LineText = Line->Text;
	NewState.LineAudio = Line->Audio;

	NewState.Choices.Empty();
	for (auto Choice : CurrentDialogueAsset->GetChoicesForLineID(LineID))
	{
		NewState.Choices.Add(FDialogueChoiceInfo(Choice.LineID, Choice.Text));
	}

	if (const UDialogueSpeakerList* SpeakerList = UYarnSettings::Get()->GetLoadedSpeakerList())
	{
		if (const FDialogueSpeaker* Speaker = SpeakerList->FindSpeakerByID(Line->SpeakerID))
		{
			NewState.SpeakerName = Speaker->DisplayName;
		}
	}

	CurrentDialogueState = NewState;

	OnRep_CurrentDialogueState();

	float LineDisplayTime;
	if (NewState.LineAudio && NewState.LineAudio->GetDuration() > 0)
	{
		MulticastPlayAudio(NewState.LineAudio);
		LineDisplayTime = NewState.LineAudio->GetDuration();
	}
	else
	{
		LineDisplayTime = CalculateDialogueDisplayTime(NewState.LineText.ToString());
	}

	UpdateDialogueCam(LineID);

	if (NewState.Choices.IsEmpty())
	{
		GetWorld()->GetTimerManager().SetTimer(AutoPlayNextLineTimerHandle, this,
		                                       &UYarnDialogueComponent::PlayNextDialogueLine,
		                                       LineDisplayTime, false);
	}
}

void UYarnDialogueComponent::HandleOptionChosen(const FName ChoiceID)
{
	GetWorld()->GetTimerManager().ClearTimer(AutoPlayNextLineTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(CameraShiftToPlayerTimerHandle);

	if (!CurrentDialogueAsset)
	{
		UE_LOG(LogYarnRuntime, Warning, TEXT("CurrentDialogueAsset is null when handling option choice."));
		StopDialogue();
		return;
	}

	const FDialogueLine* ChoiceLine = CurrentDialogueAsset->GetChoiceByID(ChoiceID);
	if (!ChoiceLine)
	{
		UE_LOG(LogYarnRuntime, Error, TEXT("Choice with ID %s not found in asset %s when handling option choice."),
		       *ChoiceID.ToString(),
		       *CurrentDialogueAsset->GetName());
		StopDialogue();
		return;
	}

	FCurrentDialogueState PlayerChoiceState;
	PlayerChoiceState.BranchID = CurrentDialogueState.BranchID;
	PlayerChoiceState.LineID = ChoiceID;
	PlayerChoiceState.SpeakerID = "Player";
	PlayerChoiceState.SpeakerName = FText::FromString("Player");
	PlayerChoiceState.LineText = ChoiceLine->Text;
	PlayerChoiceState.LineAudio = ChoiceLine->Audio;
	PlayerChoiceState.Choices.Empty();

	CurrentDialogueState = PlayerChoiceState;

	OnRep_CurrentDialogueState();
	
	float PlayerChoiceDisplayTime;
	if (PlayerChoiceState.LineAudio && PlayerChoiceState.LineAudio->GetDuration() > 0)
	{
		MulticastPlayAudio(PlayerChoiceState.LineAudio);
		PlayerChoiceDisplayTime = PlayerChoiceState.LineAudio->GetDuration();
	}
	else
	{
		PlayerChoiceDisplayTime = CalculateDialogueDisplayTime(PlayerChoiceState.LineText.ToString());
	}

	const FName JumpToBranchID = ChoiceLine->JumpToBranchID;
	GetWorld()->GetTimerManager().SetTimer(AutoPlayNextLineTimerHandle, [this, JumpToBranchID]()
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoPlayNextLineTimerHandle);

		if (JumpToBranchID.IsNone())
		{
			PlayNextDialogueLine();
			return;
		}

		const FDialogueBranch* Branch = CurrentDialogueAsset->GetBranchByID(JumpToBranchID);
		if (!Branch || Branch->Lines.IsEmpty())
		{
			UE_LOG(LogYarnRuntime, Error, TEXT("Invalid branch jump from choice."));
			StopDialogue();
			return;
		}

		const FName LineID = Branch->Lines[0].LineID;
		HandlePlayDialogueLine(JumpToBranchID, LineID);
	}, PlayerChoiceDisplayTime, false);
}

void UYarnDialogueComponent::HandleStopDialogue()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoPlayNextLineTimerHandle);

	CurrentDialogueAsset = nullptr;
	CurrentDialogueState = FCurrentDialogueState();

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			if (DialogueWidgetInstance)
			{
				DialogueWidgetInstance->RemoveFromParent();
				DialogueWidgetInstance = nullptr;
			}
			PC->SetViewTargetWithBlend(OwnerPawn, 0.f);
		}
	}

	if (GetOwnerRole() == ROLE_Authority && DialogueCam)
	{
		DialogueCam->Destroy();
		DialogueCam = nullptr;
	}

	OnDialogueEnded.Broadcast();
}

void UYarnDialogueComponent::UpdateDialogueCam(const FName DialogueLineID)
{
	if (!DialogueCam || !CurrentDialogueAsset)
		return;

	auto ResolveActorByID = [&](const FName ID) -> AActor*
	{
		if (ID == NAME_None) return nullptr;

		if (ID == "Player")
		{
			return GetOwner();
		}

		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			if (It->ActorHasTag(ID))
			{
				return *It;
			}
		}
		return nullptr;
	};

	const FDialogueLine* Line = CurrentDialogueAsset->GetLineByID(DialogueLineID);
	if (!Line) return;

	FName ResolvedListenerID = NAME_None;

	if (Line->SpeakerID == "Player")
	{
		if (const FDialogueBranch* Branch = CurrentDialogueAsset->GetBranchByID(UYarnDialogueAsset::GetMainBranchID()))
		{
			const int32 CurrentIndex = Branch->Lines.IndexOfByPredicate([&](const FDialogueLine& L)
			{
				return L.LineID == DialogueLineID;
			});
			for (int32 i = CurrentIndex + 1; i < Branch->Lines.Num(); ++i)
			{
				if (Branch->Lines[i].SpeakerID != "Player")
				{
					ResolvedListenerID = Branch->Lines[i].SpeakerID;
					break;
				}
			}
		}
	}
	else
	{
		ResolvedListenerID = "Player";
	}

	AActor* SpeakerActor = ResolveActorByID(Line->SpeakerID);
	AActor* ListenerActor = ResolveActorByID(ResolvedListenerID);

	DialogueCam->MoveToCenterOfActors({ResolveActorByID(ResolvedListenerID), ResolveActorByID(Line->SpeakerID)});
	DialogueCam->MoveCameraForDialogue(SpeakerActor, ListenerActor);

	if (!Line->PlayerChoiceIDs.IsEmpty())
	{
		GetWorld()->GetTimerManager().SetTimer(CameraShiftToPlayerTimerHandle, [this, SpeakerActor, ListenerActor]()
		{
			DialogueCam->MoveCameraForDialogue(ListenerActor, SpeakerActor);
		}, CalculateDialogueDisplayTime(CurrentDialogueState.LineText.ToString()), false);
	}
}

void UYarnDialogueComponent::OnOptionChosen(const FName ChoiceID)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerOptionChosen(ChoiceID);
	}
	else
	{
		HandleOptionChosen(ChoiceID);
	}
}

float UYarnDialogueComponent::CalculateDialogueDisplayTime(const FString& DialogueLineText)
{
	const float BaseWordsPerMinute = UYarnSettings::Get()->BaseWordsPerMinute;
	const float MinimumDialogueDisplayTime = UYarnSettings::Get()->MinimumDialogueDisplayTime;

	auto CountWords = [](const FString& InString) -> int32
	{
		TArray<FString> Words;
		InString.ParseIntoArray(Words, TEXT(" "), true);
		return Words.Num();
	};

	const int32 WordCount = CountWords(DialogueLineText);

	const float SecondsPerWord = 60.0f / BaseWordsPerMinute;
	const float CalculatedTime = WordCount * SecondsPerWord;

	return FMath::Max(CalculatedTime, MinimumDialogueDisplayTime);
}

void UYarnDialogueComponent::ServerStartDialogue_Implementation(UYarnDialogueAsset* InDialogueAsset)
{
	HandleStartDialogue(InDialogueAsset);
}

bool UYarnDialogueComponent::ServerStartDialogue_Validate(UYarnDialogueAsset* InDialogueAsset)
{
	return ValidateDialogueAsset(InDialogueAsset);
}

void UYarnDialogueComponent::ServerPlayDialogueLine_Implementation(FName BranchID, FName LineID)
{
	HandlePlayDialogueLine(BranchID, LineID);
}

bool UYarnDialogueComponent::ServerPlayDialogueLine_Validate(FName BranchID, FName LineID)
{
	if (!ValidateDialogueAsset(CurrentDialogueAsset))
		return false;

	const FDialogueBranch* Branch = CurrentDialogueAsset->GetBranchByID(BranchID);
	if (!Branch)
		return false;

	const FDialogueLine* Line = CurrentDialogueAsset->GetLineByID(LineID);
	return (Line != nullptr);
}
