// All rights reserved Dominik Pavlicek


#include "MPSPlayerController.h"
#include "MPShooterGameMode.h"
#include "Net/UnrealNetwork.h"
#include "MPShooterCharacter.h"

AMPSPlayerController::AMPSPlayerController()
{
	TeamID = ETeamID::ETI_Default;
}

void AMPSPlayerController::ClientUpdateInGameUI_Implementation()
{
	OnUpdateInGameUI();
}

void AMPSPlayerController::InitializeTeam(ETeamID& TeamID)
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

void AMPSPlayerController::OnRep_TeamID()
{
	ClientSetTeamColour();
}

void AMPSPlayerController::ClientSetTeamColour_Implementation()
{
	if (AMPShooterCharacter* ControlledPawn = Cast<AMPShooterCharacter>(GetPawn()))
	{
		{
			ControlledPawn->SetTeamColour();
		}
	}
}

void AMPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPSPlayerController, TeamID);
}