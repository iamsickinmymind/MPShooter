// All rights reserved Dominik Pavlicek


#include "MPSPlayerController.h"
#include "MPShooterGameMode.h"
#include "Net/UnrealNetwork.h"
#include "MPShooterCharacter.h"

AMPSPlayerController::AMPSPlayerController()
{
	TeamID = ETeamID::ETI_Default;
	RespawnDealy = 5.f;
}

void AMPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (AMPShooterGameMode* GM = Cast<AMPShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		InitializeTeam(TeamID);
	}
}

void AMPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Respawn", IE_Pressed, this, &AMPSPlayerController::Respawn);
}

void AMPSPlayerController::ClientUpdateInGameUI_Implementation()
{
	OnUpdateInGameUI();
}

void AMPSPlayerController::InitializeTeam(ETeamID& InTeamID)
{
	if (AMPShooterGameMode* GM = Cast<AMPShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->SetTeamID(this);
	}
}

void AMPSPlayerController::SetTeam(const ETeamID NewTeamID)
{
	if (NewTeamID != TeamID)
	{
		TeamID = NewTeamID;
	}
}

void AMPSPlayerController::Respawn()
{
	if (AMPShooterCharacter* PlayerPawn = Cast<AMPShooterCharacter>(GetPawn()))
	{
		if (PlayerPawn->IsAlive())
		{
			return;
		}
	}

	ChangeState(NAME_Inactive);

	if (!HasAuthority())
	{
		ServerRespawn();
	}
	else
	{
		RespawnDealy += RespawnDealyPenalty;
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, this, &AMPSPlayerController::ServerRestartPlayer, RespawnDealy, false);
	}
}

void AMPSPlayerController::ServerRespawn_Implementation()
{
	Respawn();
}

void AMPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPSPlayerController, TeamID);
	DOREPLIFETIME(AMPSPlayerController, RespawnDealy);
	DOREPLIFETIME(AMPSPlayerController, RespawnDealyPenalty);
}