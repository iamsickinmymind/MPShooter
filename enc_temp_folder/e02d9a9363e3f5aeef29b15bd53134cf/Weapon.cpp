// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "MPSPlayerController.h"
#include "MPShooterCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SetReplicates(true);
	SetReplicateMovement(true);
	bNetUseOwnerRelevancy = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetRootComponent());
	WeaponMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	WeaponMesh->bReceivesDecals = true;
	WeaponMesh->CastShadow = true;
	WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	CurrentWeaponState = EWeaponState::EWS_Idle;
	CurrentWeaponCategory = EWeaponType::EWT_Primary;
	WeaponClassification = EWeaponClassification::EWC_HG;

	bIsEquipped = false;
	bPendingEquip = false;
	bPendingReload = false;
	bWantsToFire = false;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

//	PostInitializeComponents instead
// 	if (HasAuthority())
// 	{
// 		AmmoInClip = WeaponAmmoSettings.MaxAmmoPerClip;
// 			AmmoInInventory = WeaponAmmoSettings.MaxAmmo;
// 	}
}

void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AmmoInClip = WeaponAmmoSettings.MaxAmmoPerClip;
	AmmoInInventory = WeaponAmmoSettings.MaxAmmo;
	TimeBetweenShots = WeaponConfig.RPM / 60;
}

void AWeapon::OnRep_WeaponOwner()
{
	/// TODO
	//  If WeaponOwner == nullptr ActivateInteraction
}

float AWeapon::PlayWeaponAnimation(UAnimMontage* Animation, float InPlayRate /*= 1.f*/, FName StartSectionName /*= NAME_Name*/)
{
	float Duration = 0.f;
	if (WeaponOwner)
	{
		if (Animation)
		{
			Duration = WeaponOwner->PlayAnimMontage(Animation, InPlayRate, StartSectionName);
		}
	}
	return Duration;
}

void AWeapon::StopWeaponAnim(UAnimMontage* AnimToPlay)
{
	if (WeaponOwner)
	{
		if (AnimToPlay)
		{
			WeaponOwner->StopAnimMontage(AnimToPlay);
		}
	}
}

bool AWeapon::CanFire() const
{
	const bool bOwnerCanFire = ((WeaponOwner != nullptr) && (Cast<AMPShooterCharacter>(WeaponOwner) && Cast<AMPShooterCharacter>(WeaponOwner)->IsAlive()));
	const bool bWeaponSateOK = (CurrentWeaponState == EWeaponState::EWS_Idle || CurrentWeaponState == EWeaponState::EWS_Firing);
	const bool bHasAmmo = AmmoInClip > 0;
	return bOwnerCanFire && bWeaponSateOK && bHasAmmo && !bPendingReload;
}

void AWeapon::StartFire()
{
	if (Role < ROLE_Authority) 
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Star fire called"))
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Star fire called"))
	}
}

void AWeapon::StopFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Stop fire called"))
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Stop fire called"))
	}
}

void AWeapon::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::EWS_Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload())
			{
				NewState = EWeaponState::EWS_Reloading;
			}
			else
			{
				NewState = CurrentWeaponState;
			}
		}
		else if (!bPendingReload)
		{
			if (bWantsToFire)
			{
				if (CanFire())
				{
					NewState = EWeaponState::EWS_Firing;
				}
			}
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::EWS_Equipping;
	}

	SetWeaponState(NewState);
}

void AWeapon::SetWeaponState(EWeaponState NewWeapoState)
{
	const EWeaponState PreviousState = CurrentWeaponState;

	if (PreviousState == EWeaponState::EWS_Firing && NewWeapoState != EWeaponState::EWS_Firing)
	{
		// Stop bursting
	}

	CurrentWeaponState = NewWeapoState;

	if (PreviousState != EWeaponState::EWS_Firing && NewWeapoState == EWeaponState::EWS_Firing)
	{
		// start blasting
	}
}

void AWeapon::AttachToPawn(EWeaponType Category)
{
	if (WeaponOwner)
	{
		DetachFromPawm();

		if (USkeletalMeshComponent* PawnMesh = WeaponOwner->GetMesh())
		{
			WeaponMesh->AttachToComponent
			(
				PawnMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				Category == EWeaponType::EWT_Primary ? WeaponSocketing.EquippedSocket : WeaponSocketing.IdleSocket
			);
		}
	}
}

void AWeapon::DetachFromPawm()
{
	WeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void AWeapon::OnEquip(EWeaponType Category)
{
	if (!WeaponOwner)
	{
		return;
	}

	bPendingEquip = true;
	DetermineWeaponState();
	CurrentWeaponCategory = Category;

	if (WeaponAnim.bPlayEquipAnim)
	{
		UAnimMontage* AnimToPlay = WeaponOwner->IsLocallyControlled() ? WeaponAnim.EquipAnim1P : WeaponAnim.EquipAnim3P;
		if (AnimToPlay)
		{
			float AnimDuration = PlayWeaponAnimation(AnimToPlay);

			if (AnimDuration <= 0.f)
			{
				AnimDuration = WeaponAnim.NoAnimEquipDuration;
			}
			EquipStartedTime = GetWorld()->GetTimeSeconds();

			EquipDuration = AnimDuration;

			GetWorldTimerManager().SetTimer(TimerHandle_EquipFinishedWeapon, this, &AWeapon::OnEquipFinished, AnimDuration, false);
		}
		else
		{
			OnEquipFinished();
		}
	}
	else
	{
		OnEquipFinished();
	}
}

void AWeapon::OnEquipFinished()
{
	AttachToPawn(CurrentWeaponCategory);

	bIsEquipped = true;
	bPendingEquip = false;

	DetermineWeaponState();

	RefreshOwnerUI();
}

void AWeapon::RefreshOwnerUI()
{
	if (WeaponOwner)
	{
		if (AMPSPlayerController* PlayerCon = Cast<AMPSPlayerController>(WeaponOwner->GetController()))
		{
			PlayerCon->ClientUpdateInGameUI();
		}
	}
}

void AWeapon::OnUnEquip()
{
	ServerOnUnEquip();
}

void AWeapon::ServerOnUnEquip_Implementation()
{
	Destroy();
}

bool AWeapon::ServerOnUnEquip_Validate()
{
	return true;
}

void AWeapon::ConsumeAmmo(const int32 AmmoToConsume /*= 1*/)
{
	const int32 AmmoDelta = FMath::Min(AmmoInClip - AmmoToConsume, 0);

	if (AmmoDelta > 0)
	{
		AmmoInClip -= AmmoToConsume;
	}
}

void AWeapon::StartReload(bool bFromReplication /*= false*/)
{
	if (!bFromReplication && !HasAuthority())
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		if (WeaponAnim.bPlayReloadAnim)
		{
			UAnimMontage* AnimToPlay = WeaponOwner->IsLocallyControlled() ? WeaponAnim.ReloadAnim1P : WeaponAnim.ReloadAnim3P;
			if (AnimToPlay)
			{
				float AnimDuration = PlayWeaponAnimation(AnimToPlay);

				if (AnimDuration <= 0.f)
				{
					AnimDuration = WeaponAnim.NoAnimReloadDuration;
				}

				GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AWeapon::StopSimulateReload, AnimDuration, false);
				if (HasAuthority())
				{
					GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
				}
			}
			else
			{
				ReloadWeapon();
			}
		}
		else
		{
			ReloadWeapon();
		}
	}
}

void AWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		/* By passing true we do not push back to server and execute it locally */
		StartReload(true);
	}
	else
	{
		StopSimulateReload();
	}
}

void AWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

bool AWeapon::ServerStartReload_Validate()
{
	return true;
}

void AWeapon::StopSimulateReload()
{
	if (CurrentWeaponState == EWeaponState::EWS_Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		if (UAnimMontage* AnimToPlay = WeaponOwner->IsLocallyControlled() ? WeaponAnim.ReloadAnim1P : WeaponAnim.ReloadAnim3P)
		{
			StopWeaponAnim(AnimToPlay);
		}
	}
}

void AWeapon::ReloadWeapon()
{
	int32 ClipDelta = FMath::Min(WeaponAmmoSettings.MaxAmmoPerClip - AmmoInClip, AmmoInInventory - AmmoInClip);

	if (ClipDelta > 0)
	{
		AmmoInClip += ClipDelta;
		AmmoInInventory -= ClipDelta;
	}

	RefreshOwnerUI();
}

void AWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool AWeapon::ServerStopFire_Validate()
{
	return true;
}

void AWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AWeapon::ServerStartFire_Validate()
{
	return true;
}

bool AWeapon::CanReload() const
{
	return ((AmmoInClip >= WeaponAmmoSettings.MaxAmmoPerClip) || (GetAmmoInInventory() > 0)) && (WeaponOwner != nullptr);
}

void AWeapon::SetWeaponOwner(ACharacter* NewOwner)
{
	if (NewOwner && NewOwner != WeaponOwner)
	{
		WeaponOwner = NewOwner;
		SetOwner(WeaponOwner);

		OnRep_WeaponOwner();
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponOwner);

	DOREPLIFETIME_CONDITION(AWeapon, AmmoInClip, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWeapon, AmmoInInventory, COND_OwnerOnly);
}
