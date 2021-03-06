// All rights reserved Dominik Pavlicek

#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "MPSPlayerController.h"
#include "MPShooterCharacter.h"
#include "MPShooter.h"
#include "Components/BoxComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SetReplicates(true);
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
	SetRootComponent(WeaponMesh);

	BoxComp	= CreateDefaultSubobject<UBoxComponent>(FName("BoxComp"));
	BoxComp->SetupAttachment(GetRootComponent());
	BoxComp->SetBoxExtent(FVector(48.f, 32.f, 32.f));
	BoxComp->SetCanEverAffectNavigation(false);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxComp->SetActive(false);

	CurrentWeaponState = EWeaponState::EWS_Idle;
	CurrentWeaponCategory = EWeaponType::EWT_Primary;
	WeaponClassification = EWeaponClassification::EWC_HG;

	bIsEquipped = false;
	bPendingEquip = false;
	bPendingReload = false;
	bWantsToFire = false;
}

FName AWeapon::GetCameraAimSocket() const
{
	return WeaponSocketing.DefaultAimCameraSocket;
}

FTransform AWeapon::GetCameraAimSocketTransform() const
{
	if (WeaponMesh)
	{
		if (WeaponMesh)
		{
			if (WeaponMesh->DoesSocketExist(WeaponSocketing.DefaultAimCameraSocket))
			{
				return WeaponMesh->GetSocketTransform(WeaponSocketing.DefaultAimCameraSocket);
			}
		}
	}
	return FTransform();
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
	TimeBetweenShots = 60 / WeaponConfig.RPM;

	switch (WeaponConfig.WeaponFireMode)
	{
	case EWeaponFireMode::EWM_SemiAuto:
		TimeBetweenShots = 0;
		break;
	case EWeaponFireMode::EWM_Burst:
		TimeBetweenShots = 0;
		break;
	default:
		break;
	}
}

void AWeapon::OnRep_WeaponOwner()
{
	/// TODO
	//  If WeaponOwner == nullptr ActivateInteraction
}

void AWeapon::OnRep_BurstCounter()
{

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
}

void AWeapon::FireWeapon()
{
	const FVector TraceStart = GetTraceLoc();
	const FVector TraceDir = GetTraceDir();
	const FVector TraceEnd = TraceStart + (TraceDir * WeaponConfig.Range);

	FHitResult Impact = WeaponTrace(TraceStart, TraceEnd);

	HandleHit(Impact, TraceStart, TraceDir);

	OnFired(TraceStart, TraceEnd, Impact.ImpactPoint);
}

FHitResult AWeapon::WeaponTrace(const FVector &TraceStart, const FVector &TraceEnd) const
{
	FCollisionQueryParams TraceParams(TEXT("WeaponTrace"), true, GetInstigator());
		TraceParams.bReturnPhysicalMaterial = true;
		TraceParams.AddIgnoredActor(this);
		TraceParams.AddIgnoredActor(WeaponOwner);

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, COLLISION_WEAPON, TraceParams);

	return Hit;
}

FVector AWeapon::GetTraceLoc() const
{
	if (WeaponMesh)
	{
		if (WeaponMesh->DoesSocketExist(WeaponSocketing.DefaultFireSocket))
		{
			return WeaponMesh->GetSocketLocation(WeaponSocketing.DefaultFireSocket);
		}
	}

	return FVector::ZeroVector;
}

FVector AWeapon::GetTraceDir() const
{
	if (WeaponMesh)
	{
		if (WeaponMesh->DoesSocketExist(WeaponSocketing.DefaultFireSocket))
		{
			return WeaponMesh->GetSocketRotation(WeaponSocketing.DefaultFireSocket).Vector();
		}
	}

	return FVector::ZeroVector;
}

void AWeapon::HandleHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
	if (Impact.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit actor %s"), *Impact.GetActor()->GetName());
	}

	ServerHandleHit(Impact, ShootDir);

	if (WeaponOwner && Impact.GetActor())
	{
		// HUD - show hit marker
	}
}

bool AWeapon::CanHit(const FHitResult& Impact)
{
	if (WeaponOwner && Impact.GetActor())
	{
		AMPShooterCharacter* HitActor = Cast<AMPShooterCharacter>(Impact.GetActor());
		AMPShooterCharacter* PawnOwner = Cast<AMPShooterCharacter>(WeaponOwner);
		if (HitActor && PawnOwner)
		{
			AMPSPlayerController* HitActorCon = Cast<AMPSPlayerController>(HitActor->GetController());
			AMPSPlayerController* PawnOwnerCon = Cast<AMPSPlayerController>(PawnOwner->GetController());
			if (HitActorCon && PawnOwnerCon)
			{
				if (HitActorCon->GetTeamID() != PawnOwnerCon->GetTeamID())
				{
					return true;
				}
			}
			else if (!HitActorCon)
			{
				return true;
			}
		}
	}
	return false;
}

void AWeapon::ServerHandleHit_Implementation(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir)
{
	if (WeaponOwner)
	{
		if (!CanHit(Impact)) return;
		float ActualHitDamage = WeaponConfig.BaseDamage;
		for (auto DamageMultiplier : WeaponConfig.DamageMultiplier)
		{
			if (DamageMultiplier.Key == Impact.BoneName)
			{
				ActualHitDamage *= DamageMultiplier.Value;
				break;
			}
		}
		///TODO @real burting over time
		// This is just a placeholder that will be removed and re[;aced with actual burst fire method
		// Maybe FireWeaponBurst();
		if (WeaponConfig.WeaponFireMode == EWeaponFireMode::EWM_Burst)
		{
			ActualHitDamage *= WeaponAmmoSettings.AmmoPerShot;
		}
		if (AActor* HitActor = Impact.GetActor())
		{
			FDamageEvent DamageEvent;
			DamageEvent.DamageTypeClass = WeaponConfig.DamageType;
			//UGameplayStatics::ApplyDamage(HitActor, ActualHitDamage, WeaponOwner->GetController(), this, WeaponConfig.DamageType);
			HitActor->TakeDamage(ActualHitDamage, DamageEvent, WeaponOwner->GetController(), this);
			//UGameplayStatics::ApplyPointDamage(HitActor, ActualHitDamage, (Impact.TraceStart - Impact.TraceEnd).GetSafeNormal(), Impact, WeaponOwner->GetController(), this, WeaponConfig.DamageType);
		}
	}
}

bool AWeapon::ServerHandleHit_Validate(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir)
{
	return true;
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
		else if (!bPendingReload && bWantsToFire && CanFire())
		{
			NewState = EWeaponState::EWS_Firing;
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
		OnBurstFinished();
	}

	CurrentWeaponState = NewWeapoState;

	if (PreviousState != EWeaponState::EWS_Firing && NewWeapoState == EWeaponState::EWS_Firing)
	{
		OnBurstStarted();
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

	AttachToPawn(CurrentWeaponCategory);

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

	OnAttached();
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
	//ServerOnUnEquip
	if (!HasAuthority())
	{
		ServerOnUnEquip();
		BoxComp->SetActive(true);
	}
	else
	{
		BoxComp->SetActive(true);
		DetachFromPawm();
		SetWeaponOwner(nullptr);
	}
}

void AWeapon::ServerOnUnEquip_Implementation()
{
	/*Destroy();*/
	OnUnEquip();
}

bool AWeapon::ServerOnUnEquip_Validate()
{
	return true;
}

int AWeapon::ConsumeAmmo()
{
	AmmoInClip -= WeaponAmmoSettings.AmmoPerShot;
	const int32 AmmoDelta = FMath::Clamp(AmmoInClip, 0, WeaponAmmoSettings.MaxAmmoPerClip);
	AmmoInClip = AmmoDelta;

	return AmmoInClip;
}

void AWeapon::OnBurstStarted()
{
	// Start firing
	// Firing might be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && TimeBetweenShots > 0.0f &&	LastFireTime + TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeapon::HandleFiring, LastFireTime + TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AWeapon::OnBurstFinished()
{
	BurstCounter = 0;

	if (GetNetMode() != NM_DedicatedServer)
	{
		//StopSimulatingWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;
}

void AWeapon::HandleFiring()
{
	if (CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			//SimulateWeaponFire();
		}

		if (WeaponOwner && WeaponOwner->IsLocallyControlled())
		{
			FireWeapon();

			AmmoInClip = ConsumeAmmo();

			// Update firing FX on remote clients if this is called on server
			BurstCounter++;
		}
	}
	else if (CanReload())
	{
		StartReload();
	}
	else if (WeaponOwner && WeaponOwner->IsLocallyControlled())
	{
		if (GetCurrentAmmoInClip() == 0 && !bRefiring)
		{
			//PlayWeaponSound(OutOfAmmoSound);
		}

		/* Reload after firing last round */
		if (GetCurrentAmmoInClip() <= 0 && CanReload())
		{
			StartReload();
		}

		/* Stop weapon fire FX, but stay in firing state */
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}

	if (WeaponOwner && WeaponOwner->IsLocallyControlled())
	{
		if (!HasAuthority())
		{
			ServerHandleFiring();
		}

		/* Retrigger HandleFiring on a delay for automatic weapons */
		bRefiring = (CurrentWeaponState == EWeaponState::EWS_Firing && TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeapon::HandleFiring, TimeBetweenShots, false);
		}
	}

	/* Make Noise on every shot. The data is managed by the PawnNoiseEmitterComponent created in SBaseCharacter and used by PawnSensingComponent in SZombieCharacter */
	if (WeaponOwner)
	{
		WeaponOwner->MakeNoise(WeaponConfig.NoiseVolumeRange);
	}

	LastFireTime = GetWorld()->GetTimeSeconds();

	RefreshOwnerUI();
}

void AWeapon::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = CanFire();

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		const int32 AmmoDelta = FMath::Max(AmmoInClip - 1, 0);

		AmmoInClip = ConsumeAmmo();

		// Update firing FX on remote clients
		BurstCounter++;
	}
}

bool AWeapon::ServerHandleFiring_Validate()
{
	return true;
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

		UAnimMontage* AnimToPlay = WeaponOwner->IsLocallyControlled() ? WeaponAnim.ReloadAnim1P : WeaponAnim.ReloadAnim3P;
		float AnimDuration = 0.0f;
		if (AnimToPlay)
		{
			AnimDuration = PlayWeaponAnimation(AnimToPlay);
		}

		if (AnimDuration <= 0.f || !WeaponAnim.bPlayReloadAnim)
		{
			AnimDuration = WeaponAnim.NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AWeapon::StopReload, AnimDuration, false);
		if (HasAuthority())
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}
	}
}

void AWeapon::StopReload()
{
	if (CurrentWeaponState == EWeaponState::EWS_Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		if (WeaponOwner)
		{
			StopWeaponAnim(WeaponOwner->IsLocallyControlled() ? WeaponAnim.ReloadAnim1P : WeaponAnim.ReloadAnim3P);
		}
	}
}

void AWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload();
	}
	else
	{
		StopReload();
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

void AWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

bool AWeapon::ServerStopReload_Validate()
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
	return ((AmmoInClip <= WeaponAmmoSettings.MaxAmmoPerClip) && (GetAmmoInInventory() > 0)) && (WeaponOwner != nullptr);
}

void AWeapon::SetWeaponOwner(ACharacter* NewOwner)
{
	if (NewOwner != WeaponOwner)
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
	DOREPLIFETIME_CONDITION(AWeapon, bPendingReload, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, BurstCounter, COND_SkipOwner);
}
