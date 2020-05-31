// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MPShooterGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "MPSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AMPShooterGameMode::AMPShooterGameMode()
{
}

void AMPShooterGameMode::BeginPlay()
{
	//...
}

void AMPShooterGameMode::SetTeamID(AMPSPlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	int32 PlayerNum = GetWorld()->GetNumPlayerControllers();

	for (int32 i = 0; i <= PlayerNum; ++i)
	{
		if (UGameplayStatics::GetPlayerController(GetWorld(), i))
		{
			if (AMPSPlayerController* PC = Cast<AMPSPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), i)))
			{
				int32 RandomTeam = FMath::RandRange(0, 1);
				switch (RandomTeam)
				{
				case 0:
					PC->SetTeam(ETeamID::ETI_TeamA);
					break;
				case 1:
					PC->SetTeam(ETeamID::ETI_TeamB);
					break;
				default:
					break;
				}
			}
		}
	}
}