// All rights reserver Dominik Pavlicek

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MPSLobbyPawn.generated.h"

class UAnimationAsset;
class UCapsuleComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class USpringArmComponent;

UCLASS()
class MPSHOOTER_API AMPSLobbyPawn : public APawn
{
	GENERATED_BODY()

public:
	AMPSLobbyPawn();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby Mesh")
	UCapsuleComponent* CapsuleComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby Mesh")
	USkeletalMeshComponent* Mesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby Mesh")
	USpringArmComponent* SpringArmComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby Mesh")
	UCameraComponent* CameraComp = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby Mesh")
	UAnimationAsset* AnimToPlay = nullptr;
};
