// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MPShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

AMPShooterCharacter::AMPShooterCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200;
	GetCharacterMovement()->JumpZVelocity = 620;
	GetCharacterMovement()->AirControl = 0.f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false;
}

void AMPShooterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMPShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMPShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMPShooterCharacter::OnStartFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMPShooterCharacter::StartReload);
	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &AMPShooterCharacter::SwitchWeapon);
}

void AMPShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (DefaultPrimaryWeapon)
		{
			const FRotator SpawnRotation = GetActorRotation();
			const FVector SpawnLocation = GetActorLocation();
			FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = SpawnParams.Instigator = this;
				SpawnParams.bNoFail = true;
			ActiveWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultPrimaryWeapon, SpawnLocation, SpawnRotation, SpawnParams);

			if (ActiveWeapon)
			{
				SetActiveWeapon(ActiveWeapon, nullptr);
			}
		}

		OnRep_Weapon();
	}
}

void AMPShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMPShooterCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMPShooterCharacter::SetSprinting(bool NewIsSprinting)
{

}

bool AMPShooterCharacter::IsAlive() const
{
	return true;
}

bool AMPShooterCharacter::IsSprinting() const
{
	return true;
}

void AMPShooterCharacter::OnRep_Weapon()
{
	SetActiveWeapon(ActiveWeapon, PreviousWeapon);
}

void AMPShooterCharacter::SetActiveWeapon(AWeapon*NewWeapon, AWeapon* LastWeapon /*= nullptr*/)
{
	PreviousWeapon = LastWeapon;

	AWeapon* LocalLastWeapon = nullptr;
	if (LastWeapon)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != ActiveWeapon)
	{
		LocalLastWeapon = ActiveWeapon;
	}

	EWeaponType PreviousCategory = EWeaponType::EWT_Primary;

	bool bHasPreviousWeapon = false;
	if (LocalLastWeapon)
	{
		PreviousCategory = LocalLastWeapon->CurrentWeaponCategory;
		LocalLastWeapon->Destroy(); // maybe dont destroy and manage to perform some actions?
		PreviousWeapon->Destroy();
		bHasPreviousWeapon = true;
	}

	ActiveWeapon = NewWeapon;

	if (NewWeapon)
	{
		NewWeapon->SetWeaponOwner(this);
		NewWeapon->OnEquip(PreviousCategory);
	}
}

void AMPShooterCharacter::SwitchWeapon()
{
	if (Role < ROLE_Authority)
	{

	}
}

void AMPShooterCharacter::OnStartFire()
{
	if (IsSprinting())
	{
		SetSprinting(false);
	}

	StartFire();
}

void AMPShooterCharacter::StartFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (ActiveWeapon)
		{
			ActiveWeapon->StartFire();
		}
	}
}

void AMPShooterCharacter::ServerStartFire_Implementation()
{
	OnStartFire();
}

bool AMPShooterCharacter::ServerStartFire_Validate()
{
	return true;
}

void AMPShooterCharacter::StartReload()
{
	if (IsAlive())
	{
		if (ActiveWeapon)
		{
			ActiveWeapon->StartReload();
		}
	}
}

void AMPShooterCharacter::EquipWeapon(AWeapon* NewWeapon, EWeaponType Category /*= EWeaponCategory::EWC_Primary */)
{
	if (!HasAuthority())
	{
		ServerEquipWeapon(NewWeapon, Category);
	}
	else if (HasAuthority())
	{
		SetActiveWeapon(NewWeapon, ActiveWeapon);
		OnRep_Weapon();
	}
}

void AMPShooterCharacter::ServerEquipWeapon_Implementation(AWeapon* NewWeapon, EWeaponType Category)
{
	EquipWeapon(NewWeapon, Category);
}

bool AMPShooterCharacter::ServerEquipWeapon_Validate(AWeapon* NewWeapon, EWeaponType Category)
{
	return true;
}

void AMPShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPShooterCharacter, ActiveWeapon);
	//DOREPLIFETIME(AMPShooterCharacter, PreviousWeapon);
}