// All rights reserved Dominik Pavlicek


#include "MPSPlayerController.h"

void AMPSPlayerController::ClientUpdateInGameUI_Implementation()
{
	OnUpdateInGameUI();
}

void AMPSPlayerController::SetTeam(const int32 NewTeamID)
{
	if (NewTeamID != TeamID)
	{
		TeamID = NewTeamID;
	}
}
