// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MPSHOOTER_API AMPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateInGameUI();

	UFUNCTION(BlueprintCallable, Client, Reliable)
	void ClientUpdateInGameUI();
	
};
