// All rights reserver Dominik Pavlicek


#include "MPSLobbyController.h"

void AMPSLobbyController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = true;
}
