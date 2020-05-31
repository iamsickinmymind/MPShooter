// All rights reserved Dominik Pavlicek


#include "MPSPlayerController.h"
#include "MPShooterGameMode.h"
#include "Net/UnrealNetwork.h"
#include "MPShooterCharacter.h"

AMPSPlayerController::AMPSPlayerController()
{
	TeamID = ETeamID::ETI_Default;
}

void AMPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (AMPShooterGameMode* GM = Cast<AMPShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		InitializeTeam(TeamID);
	}
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

void AMPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPSPlayerController, TeamID);
}