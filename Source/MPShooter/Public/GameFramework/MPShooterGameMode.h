// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MPShooterGameMode.generated.h"

enum class ETeamID : uint8;

UCLASS(minimalapi)
class AMPShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMPShooterGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetTeamID(AMPSPlayerController* PlayerController);
};



