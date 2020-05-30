// All rights reserved Dominik Pavlicek

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
