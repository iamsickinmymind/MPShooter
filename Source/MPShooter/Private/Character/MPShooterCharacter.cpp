// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MPShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Weapon/Weapon.h"
#include "Weapon/FireWeapon.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "MPSPlayerController.h"
#include "MPShooter.h"

AMPShooterCharacter::AMPShooterCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200;
	GetCharacterMovement()->JumpZVelocity = 620;
	GetCharacterMovement()->AirControl = 0.f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), "CameraSocket");
	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = true;

	DefaultCameraLocation = FollowCamera ? FollowCamera->GetComponentLocation() : FVector(0);
	DefaultFOV = FollowCamera ? FollowCamera->FieldOfView : 90.0f;

	HealthComp = CreateDefaultSubobject<UHealthComponent>(FName("HealthComponent"));
}

void AMPShooterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMPShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMPShooterCharacter::MoveRight);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMPShooterCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMPShooterCharacter::StopSprint);


	PlayerInputComponent->BindAxis("Turn", this, &AMPShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMPShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMPShooterCharacter::OnStartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMPShooterCharacter::OnStopFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMPShooterCharacter::StartReload);
	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &AMPShooterCharacter::SwitchWeapon);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMPShooterCharacter::StartAiming);
	PlayerInputComponent->BindAction("AIm", IE_Released, this, &AMPShooterCharacter::StopAiming);
}

void AMPShooterCharacter::Turn(float Value)
{
	if (Value != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		PC->AddYawInput(Value);
	}
}

void AMPShooterCharacter::LookUp(float Value)
{
	if (Value != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		PC->AddPitchInput(Value);
	}
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

			ActiveWeapon = GetWorld()->SpawnActor<AFireWeapon>(DefaultPrimaryWeapon, SpawnLocation, SpawnRotation, SpawnParams);

			if (ActiveWeapon)
			{
				SetActiveWeapon(ActiveWeapon, nullptr);
			}
		}

		OnRep_Weapon();
	}
}

void AMPShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/// DODO:
	// - create bool ShouldUpdateCamera() const
	// - determine if camera should be moved
	// - execute camera mvoement only if camera should be moved

	if (IsLocallyControlled())
	{
		SetAiminingCamera(DeltaTime);
	}
}

float AMPShooterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (HasAuthority())
	{
		if (EventInstigator && DamageCauser)
		{
			if (HealthComp)
			{
				float DeltaHealth = HealthComp->ModifyHealth(-DamageAmount);
				if (DeltaHealth > 0)
				{
					// OnHit();
					UE_LOG(LogTemp, Warning, TEXT("Not ready to die"))
				}
				else
				{
					TryToDie(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
				}
			}
		}

		if (AMPSPlayerController* PC = Cast<AMPSPlayerController>(GetController()))
		{
			PC->ClientUpdateInGameUI();
		}
	}

	return DamageAmount;
}

void AMPShooterCharacter::TryToDie(const float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	if (EventInstigator != GetController())
	{
		KilledBy(DamageCauser, EventInstigator, DamageEvent);
	}
	else
	{
		KilledSelf(this, DamageEvent);
	}
}

void AMPShooterCharacter::KilledBy(const AActor* DamageCauser, const AController* EventInstigator, struct FDamageEvent const & DamageEvent)
{
	if (auto TempKiller = Cast<AMPShooterCharacter>(EventInstigator->GetPawn()))
	{
		Killer = TempKiller;
	}
	OnRep_Killer();
}

void AMPShooterCharacter::KilledSelf(const AActor* DamageCauser, struct FDamageEvent const & DamageEvent)
{
	OnRep_Killer();
}

void AMPShooterCharacter::OnRep_Killer()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMovementComponent()->SetIsReplicated(false);

	if (AMPSPlayerController* PC = Cast<AMPSPlayerController>(GetController()))
	{

		PC->DisableInput(nullptr);
	}
}

void AMPShooterCharacter::SetAiminingCamera(float DeltaTime)
{
	float DesiredFOV;
	if (ActiveWeapon)
	{
		DesiredFOV = IsAiming() ? ActiveWeapon->WeaponConfig.DefaultAimFOV : DefaultFOV;
	}
	else
	{
		DesiredFOV = DefaultFOV;
	}

	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, DesiredFOV, DeltaTime, 10.f));

	if (ActiveWeapon != nullptr) {

		const FVector ADSLocation = ActiveWeapon->GetCameraAimSocketTransform().GetLocation();
		DefaultCameraLocation = CameraBoom->GetSocketLocation(USpringArmComponent::SocketName);

		FVector CameraLocation = bIsAiming ? ADSLocation : DefaultCameraLocation;

		const float InterpSpeed = FVector::Dist(ADSLocation, DefaultCameraLocation) / ActiveWeapon->GetCemraAimTransitionSpeed();
		FollowCamera->SetWorldLocation(FMath::VInterpTo(FollowCamera->GetComponentLocation(), CameraLocation, DeltaTime, InterpSpeed));
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

void AMPShooterCharacter::StartSprint()
{
	SetSprinting(true);
}

void AMPShooterCharacter::StopSprint()
{
	SetSprinting(false);
}

void AMPShooterCharacter::SetSprinting(bool NewSprinting)
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	if (!HasAuthority())
	{
		if (bIsSprinting != NewSprinting)
		{
			bIsSprinting = NewSprinting;
		}
	}
}

void AMPShooterCharacter::ServerSetSprinting_Implementation(bool NewSprinting)
{
	SetSprinting(NewSprinting);
}

bool AMPShooterCharacter::ServerSetSprinting_Validate(bool NewSprinting)
{
	return true;
}

bool AMPShooterCharacter::IsSprinting() const
{
	return bIsSprinting;
}

bool AMPShooterCharacter::IsAiming()
{
	return bIsAiming;
}

bool AMPShooterCharacter::CanAim()
{
	return ActiveWeapon != nullptr && !IsSprinting();
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
		//PreviousWeapon->Destroy();
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

void AMPShooterCharacter::OnStopFire()
{
	StopFire();
}

void AMPShooterCharacter::StopFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (ActiveWeapon)
		{
			ActiveWeapon->StopFire();
		}
	}
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

void AMPShooterCharacter::StartAiming()
{
	SetAiming(true);
	if (!HasAuthority())
	{
		ServerStartAiming();
	}
}

void AMPShooterCharacter::StopAiming()
{
	SetAiming(false);
	if (!HasAuthority())
	{
		ServerStopAiming();
	}
}

void AMPShooterCharacter::SetAiming(const bool NewAiming)
{
	// Local aiming
	if (CanAim())
	{
		bIsAiming = NewAiming;
	}

	// Aim at server but do not COND_SkipOwner
	if (!HasAuthority())
	{
		ServerSetAiming(NewAiming);
	}
}

void AMPShooterCharacter::ServerSetAiming_Implementation(const bool NewAiming)
{
	SetAiming(NewAiming);
}

bool AMPShooterCharacter::ServerSetAiming_Validate(const bool NewAiming)
{
	return true;
}

void AMPShooterCharacter::ServerStopAiming_Implementation()
{
	SetAiming(false);
}

bool AMPShooterCharacter::ServerStopAiming_Validate()
{
	return true;
}

void AMPShooterCharacter::ServerStartAiming_Implementation()
{
	SetAiming(true);
}

bool AMPShooterCharacter::ServerStartAiming_Validate()
{
	return true;
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
	DOREPLIFETIME(AMPShooterCharacter, Killer);
	DOREPLIFETIME_CONDITION(AMPShooterCharacter, bIsAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMPShooterCharacter, bIsSprinting, COND_SkipOwner);
}