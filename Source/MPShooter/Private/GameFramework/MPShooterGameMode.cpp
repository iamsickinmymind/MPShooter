// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MPShooterGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "MPSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AMPShooterGameMode::AMPShooterGameMode()
{
	bPauseable = false;
}

void AMPShooterGameMode::SetTeamID(AMPSPlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (PlayerControllers.Find(PlayerController) > 0)
	{
		return;
	}
	
	PlayerControllers.AddUnique(PlayerController);
	int32 TeamA = 0;
	int32 TeamB = 0;

	for (auto Itr : PlayerControllers)
	{
		if (AMPSPlayerController* ItrPlayerController = Cast<AMPSPlayerController>(Itr))
		{
			ETeamID ItrTeamID = ItrPlayerController->GetTeamID();
			switch (ItrTeamID)
			{
			case ETeamID::ETI_TeamA:
				TeamA++;
				break;

			case ETeamID::ETI_TeamB:
				TeamB++;
				break;

			default:
				break;
			}
		}
	}

	if (TeamA > TeamB)
	{
		PlayerController->SetTeam(ETeamID::ETI_TeamB);
	}
	if (TeamA < TeamB)
	{
		PlayerController->SetTeam(ETeamID::ETI_TeamA);
	}
	if (TeamA == TeamB)
	{
		int32 RandomTeam = FMath::FRandRange(0, 1);
		switch (RandomTeam)
		{
		case 0:
			PlayerController->SetTeam(ETeamID::ETI_TeamA);
			break;

		case 1:
			PlayerController->SetTeam(ETeamID::ETI_TeamB);
			break;

		default:
			break;
		}
	}
}