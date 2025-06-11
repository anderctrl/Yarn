#include "YarnDialogueCameraActor.h"
#include "CineCameraComponent.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "YarnModule.h"

AYarnDialogueCameraActor::AYarnDialogueCameraActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
	AActor::SetReplicateMovement(true);

	ReplicatedFieldOfView = 90.0f;
	ReplicatedFocusSettings = FCameraFocusSettings();
	ReplicatedLookatTrackingSettings = FCameraLookatTrackingSettings();
}

void AYarnDialogueCameraActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYarnDialogueCameraActor, ReplicatedFocusSettings);
	DOREPLIFETIME(AYarnDialogueCameraActor, ReplicatedFieldOfView);
	DOREPLIFETIME(AYarnDialogueCameraActor, ReplicatedLookatTrackingSettings);
}

FVector AYarnDialogueCameraActor::GetHeadLocation(const AActor* Actor)
{
	if (!Actor) return FVector::ZeroVector;

	if (const USkeletalMeshComponent* Mesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (Mesh->DoesSocketExist(TEXT("Head")))
		{
			return Mesh->GetSocketLocation(TEXT("Head"));
		}
		return Mesh->GetComponentLocation();
	}

	FVector Origin, Extent;
	Actor->GetActorBounds(true, Origin, Extent);
	return Origin + FVector(0.f, 0.f, Extent.Z * 0.5f);
}

void AYarnDialogueCameraActor::MoveToCenterOfActors(const TArray<AActor*>& Actors)
{
	if (Actors.IsEmpty()) return;

	FVector Sum = FVector::ZeroVector;
	float MinZ = FLT_MAX, MaxZ = -FLT_MAX;
	float MaxDistance = 0.f;

	for (int32 i = 0; i < Actors.Num(); ++i)
	{
		const AActor* A = Actors[i];
		if (!A) continue;

		FVector Loc = A->GetActorLocation();
		Sum += Loc;

		FVector Origin, Extent;
		A->GetActorBounds(true, Origin, Extent);
		MinZ = FMath::Min(MinZ, Origin.Z - Extent.Z);
		MaxZ = FMath::Max(MaxZ, Origin.Z + Extent.Z);

		for (int32 j = i + 1; j < Actors.Num(); ++j)
		{
			if (const AActor* B = Actors[j])
				MaxDistance = FMath::Max(MaxDistance, FVector::Dist(Loc, B->GetActorLocation()));
		}
	}

	FVector Center = Sum / Actors.Num();
	Center.Z = (MinZ + MaxZ) * 0.8f;

	if (MaxDistance < 300.0f)
		Center += (Center - GetActorLocation()).GetSafeNormal() * 300.f;

	SetActorLocation(Center);
}

void AYarnDialogueCameraActor::MoveCameraForDialogue(AActor* SpeakerActor, AActor* ListenerActor)
{
	if (!SpeakerActor || !ListenerActor || !GetCineCameraComponent()) return;

	const FVector SpeakerHead = GetHeadLocation(SpeakerActor);
	const FVector ListenerHead = GetHeadLocation(ListenerActor);

	const FVector Direction = (SpeakerHead - ListenerHead).GetSafeNormal();
	const float Distance = FVector::Dist(SpeakerHead, ListenerHead);

	FVector CameraPos;
	if (Distance < 200.f)
	{
		const FVector Mid = (SpeakerHead + ListenerHead) * 0.5f;
		const FVector Side = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
		CameraPos = Mid + Side * 150.f;
	}
	else
	{
		CameraPos = SpeakerHead - Direction * 200.f;
	}

	CameraPos.Z = SpeakerHead.Z;

	if (FVector::Dist(GetActorLocation(), CameraPos) > 50.f)
	{
		SetActorLocation(CameraPos);
		SetActorRotation((SpeakerHead - CameraPos).Rotation());
	}

	FocusOnActor(SpeakerActor);
}

void AYarnDialogueCameraActor::FocusOnActor(AActor* InActor)
{
    if (!InActor || !GetCineCameraComponent()) return;

    if (GetLocalRole() == ROLE_Authority)
    {
        const FVector FocusPoint = GetHeadLocation(InActor);
        const FVector CameraLoc = GetActorLocation();
        const float Distance = FVector::Dist(CameraLoc, FocusPoint);
        constexpr float DesiredSize = 200.f;

        const float NewFOV = 2.f * FMath::RadiansToDegrees(FMath::Atan(DesiredSize / (2.f * Distance)));
        
        ReplicatedFieldOfView = NewFOV;

        ReplicatedFocusSettings.FocusMethod = ECameraFocusMethod::Manual;
        ReplicatedFocusSettings.ManualFocusDistance = Distance;
        ReplicatedFocusSettings.bSmoothFocusChanges = false;

        ReplicatedLookatTrackingSettings.bEnableLookAtTracking = true;
        ReplicatedLookatTrackingSettings.ActorToTrack = InActor;
        ReplicatedLookatTrackingSettings.RelativeOffset = FocusPoint - InActor->GetActorLocation();
        ReplicatedLookatTrackingSettings.LookAtTrackingInterpSpeed = 0.f;

        OnRep_FieldOfView();
        OnRep_FocusSettings();
        OnRep_LookAtTrackingSettings();

        UE_LOG(LogYarnRuntime, Verbose, TEXT("DialogueCam focused on %s | Distance: %.1f | FOV: %.1f"), *GetNameSafe(InActor), Distance, NewFOV);
    }
}

void AYarnDialogueCameraActor::OnRep_FocusSettings()
{
	if (UCineCameraComponent* CineCam = GetCineCameraComponent())
    {
        CineCam->FocusSettings = ReplicatedFocusSettings;
        CineCam->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
        CineCam->PostProcessSettings.DepthOfFieldFstop = 4.0f;
    }
}

void AYarnDialogueCameraActor::OnRep_FieldOfView()
{
	if (UCineCameraComponent* CineCam = GetCineCameraComponent())
    {
        CineCam->SetFieldOfView(ReplicatedFieldOfView);
    }
}

void AYarnDialogueCameraActor::OnRep_LookAtTrackingSettings()
{
	if (GetCineCameraComponent())
    {
        this->LookatTrackingSettings = ReplicatedLookatTrackingSettings; 
    }
}

void AYarnDialogueCameraActor::FocusOnActorByTag(const FName ActorTag)
{
	if (!ActorTag.IsValid()) return;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(ActorTag))
		{
			FocusOnActor(*It);
			break;
		}
	}
}