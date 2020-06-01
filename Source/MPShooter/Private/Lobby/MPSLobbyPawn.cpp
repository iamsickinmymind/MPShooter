// All rights reserver Dominik Pavlicek


#include "MPSLobbyPawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AMPSLobbyPawn::AMPSLobbyPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(42, 96);
	SetRootComponent(CapsuleComp);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArmComp"));
	SpringArmComp->SetupAttachment(GetRootComponent());
	SpringArmComp->SetRelativeRotation(FRotator(0.f, -180.f, 0.f));
	SpringArmComp->TargetArmLength = 600.f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(FName("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("SkeletalMesh"));
	Mesh->SetupAttachment(GetRootComponent());
}

void AMPSLobbyPawn::BeginPlay()
{
	Super::BeginPlay();

	if (Mesh)
	{
		if (AnimToPlay)
		{
			Mesh->PlayAnimation(AnimToPlay, true);
		}
	}
}

void AMPSLobbyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMPSLobbyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

