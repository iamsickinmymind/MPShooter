// All rights reserver Dominik Pavlicek

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPSLobbyController.generated.h"

/**
 * 
 */
UCLASS()
class MPSHOOTER_API AMPSLobbyController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	virtual void BeginPlay() override;

	void TryJoinSession();
};
