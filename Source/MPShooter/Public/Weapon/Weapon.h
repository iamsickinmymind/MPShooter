// All rights reserved Dominik Pavlicek

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UAnimMontage;
class UBoxComponent;
class UTexture2D;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Idle		UMETA(DisplayName = "Idle"),
	EWS_Equipping	UMETA(DisplayName = "Equipped"),
	EWS_Reloading	UMETA(DisplayName = "Reloading"),
	EWS_Firing		UMETA(DisplayName = "Firing"),

	EWS_Default		UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Primary		UMETA(DisplayName = "Primary"),
	EWT_Secondary	UMETA(DisplayName = "Secondary"),

	EWT_Default		UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWeaponClassification : uint8
{
	EWC_SMG		UMETA(DisplayName = "SMG"),
	EWC_AR		UMETA(DisplayName = "Assault Rifle"),
	EWC_HG		UMETA(DisplayName = "Handgun"),
	EWC_SR		UMETA(DisplayName = "Sniper Rifle"),
	EWC_SG		UMETA(DisplayName = "Shotgun"),
	EWC_LMG		UMETA(DisplayName = "LMG"),

	EWC_Default UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	EWM_SemiAuto UMETA(DisplayName = "Semi Auto"),
	EWM_FullAuto UMETA(DisplayName = "Full Auto"),
	EWM_Burst	UMETA(DisplayName = "Burst")
};

USTRUCT(BlueprintType)
struct FWeaponConfig
{

	GENERATED_BODY()

	FWeaponConfig()
	{
		Range = 30000.f;
		BaseDamage = 20.f;
		WeaponFireMode = EWeaponFireMode::EWM_SemiAuto;
		DamageMultiplier.Add(FName("head"), 2.f);
		DefaultAimFOV = 60.f;
		RPM = 800.f;
		NoiseVolumeRange = 1.f;
		CameraAimTransitionSpeed = 1.f;
	}

public:

	UPROPERTY(EditDefaultsOnly)
	FName WeaponDisplayName;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* WeaponIcon = nullptr;

	/**Determines the effective fire range.*/
	UPROPERTY(EditDefaultsOnly)
	float Range;

	/** Rounds per minut.
	How many shots can this weapon fire per minute.*/
	UPROPERTY(EditDefaultsOnly)
	float RPM;

	/**Values from 0.1 to 1.
	Lower values can be achieved with supressores.*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMax = 1, UIMax = 1, ClampMin = 0.1, UIMin = 0.1))
	float NoiseVolumeRange;

	/**Determines default FOV when aiming.
	Is overwritten by Scope attachment FOV.*/
	UPROPERTY(EditDefaultsOnly)
	float DefaultAimFOV;

	/**Set how fast can camera aiming to aim position.*/
	UPROPERTY(EditDefaultsOnly)
	float CameraAimTransitionSpeed;

	/**Determines the fire mode of the weapon.*/
	UPROPERTY(EditDefaultsOnly)
	EWeaponFireMode WeaponFireMode;

	/**The type of damage this weapon deals.*/
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDamageType>DamageType;

	/**How much damage does weapon deal per hit.*/
	UPROPERTY(EditDefaultsOnly)
	float BaseDamage;

	/**Allows to make a damage multiplication per hit bone.*/
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, float> DamageMultiplier;
};

USTRUCT(BlueprintType)
struct FWeaponAmmo
{
	GENERATED_BODY()

	FWeaponAmmo()
	{
		MaxAmmo = 90;
		MaxAmmoPerClip = 30;
		AmmoPerShot = 1;
	}

public:

	/**Defines the ammonution type used by weapon*/
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AAmmo> AmmoType;

	/**How much of total ammo is a single player allowed to carry.*/
	UPROPERTY(EditDefaultsOnly, meta =(UIMin = 0, ClampMin = 0))
	int32 MaxAmmo;

	/**How much ammo can be loaded in a single clip.*/
	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmoPerClip;

	/**How much ammo is consumed per shot*/
	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 1, ClampMin = 1))
	int32 AmmoPerShot;
};

USTRUCT(BlueprintType)
struct FWeaponSocket
{
	GENERATED_BODY()

	FWeaponSocket() 
	{
		AttachSocket = FName("Weapon_Right");
		IdleSocket = FName("Weapon_Idle");
		EquippedSocket = FName("Weapon_Right");
		DefaultAimCameraSocket = FName("DefaultAimCameraSocket");
		DefaultFireSocket = FName("MuzzleSocket");
	}

public:

	/**Socket name of the player WeaponMesh to which is the weapon attached.*/
	/**If Uquipped, Socket is EquippedSocket. Otherwise IdleSocket.*/
	UPROPERTY(VisibleAnywhere)
	FName AttachSocket;

	/**Default name of equipped socket.*/
	UPROPERTY(EditDefaultsOnly)
	FName EquippedSocket;

	/**Default name of Idle socket.*/
	UPROPERTY(EditDefaultsOnly)
	FName IdleSocket;

	/**Default name of camera socket used for aiming.
	Has lower priority than Scope camera socket.*/
	UPROPERTY(EditDefaultsOnly)
	FName DefaultAimCameraSocket;

	/**Default name of muzzle socket from which is calculated FireStartLocation.
	Has lower priority than WeaponAttachment Muzzle.*/
	UPROPERTY(EditDefaultsOnly)
	FName DefaultFireSocket;
};

USTRUCT(BlueprintType)
struct FWeaponAnim
{
	GENERATED_BODY()

	FWeaponAnim()
	{
		bPlayReloadAnim = true;
		bPlayFireAnim = true;
		bPlayEquipAnim = true;
		NoAnimReloadDuration = 1.5f;
		NoAnimEquipDuration = 1.5f;
		NoAnimFireDuration = 0.25f;
	}

public:

	UPROPERTY(EditDefaultsOnly)
	bool bPlayFireAnim;

	/* Time to assign on fire when no animation is found */
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "!bPlayFireAnim"))
	float NoAnimFireDuration;

	/**Anim played from players view if PlayerMode is FPP*/
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPlayFireAnim"))
	UAnimMontage* FireAnim1P = nullptr;

	/**Anim played from players view if PlayerMode is TPP*/
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPlayFireAnim"))
	UAnimMontage* FireAnim3P = nullptr;

	UPROPERTY(EditDefaultsOnly)
	bool bPlayReloadAnim;

	/* Time to assign on reload when no animation is found */
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "!bPlayReloadAnim"))
	float NoAnimReloadDuration;

	/**Anim played from players view if PlayerMode is FPP*/
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPlayReloadAnim"))
	UAnimMontage* ReloadAnim1P = nullptr;

	/**Anim played from players view if PlayerMode is TPP*/
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPlayReloadAnim"))
	UAnimMontage* ReloadAnim3P = nullptr;

	UPROPERTY(EditDefaultsOnly)
	bool bPlayEquipAnim;

	/* Time to assign on equip when no animation is found */
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "!bPlayEquipAnim"))
	float NoAnimEquipDuration;

	/**Anim played from players view if PlayerMode is FPP*/
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPlayReloadAnim"))
	UAnimMontage* EquipAnim1P = nullptr;

	/**Anim played from players view if PlayerMode is TPP*/
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPlayReloadAnim"))
	UAnimMontage* EquipAnim3P = nullptr;
};

USTRUCT(BlueprintType)
struct FWeaponAttachments
{

	GENERATED_BODY()

	FWeaponAttachments()
	{
		ScopeSocket = FName("ScopeSocket");
		StockSocket = FName("StockSocket");
		GripSocket = FName("GripSocket");
		BarrelSocket = FName("BarrelSocket");
		MuzzleSocket = FName("MuzzleSocket");
	}

public:

	UPROPERTY(EditDefaultsOnly)
	FName ScopeSocket;

	UPROPERTY(EditDefaultsOnly)
	FName StockSocket;

	UPROPERTY(EditDefaultsOnly)
	FName GripSocket;

	UPROPERTY(EditDefaultsOnly)
	FName BarrelSocket;

	UPROPERTY(EditDefaultsOnly)
	FName MuzzleSocket;
};

UCLASS(NotBlueprintable)
class MPSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()

	friend class AMPShooterCharacter;
	
public:	
#pragma region public

	AWeapon();

	UFUNCTION(BlueprintImplementableEvent)
	void OnAttached();

	UFUNCTION(BlueprintImplementableEvent)
	void OnFired(const FVector &TraceStart, const FVector &TraceEnd, const FVector &ImpactPoint);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "Wweapon")
	void SetWeaponOwner(ACharacter* NewOwner);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE	ACharacter* GetWeaponOwner() const { return WeaponOwner; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE FName GetAttachSocket() const { return WeaponSocketing.AttachSocket; };

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE FName GetWeaponName() const { return WeaponConfig.WeaponDisplayName; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE UTexture2D* GetWeaponIcon() const { return WeaponConfig.WeaponIcon; };

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE FName GetDefaultAimCameraSocket() const { return WeaponSocketing.DefaultAimCameraSocket; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE EWeaponState GetWeaponState() const { return CurrentWeaponState; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE int32 GetCurrentAmmoInClip() const { return AmmoInClip; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE int32 GetAmmoInInventory()const { return AmmoInInventory; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE EWeaponType GetWeaponCategory() const {return CurrentWeaponCategory;};

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE EWeaponClassification GetWeaponClassification() const { return WeaponClassification; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE EWeaponFireMode GetWeaponFiremMode() const { return WeaponConfig.WeaponFireMode; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE float GetCemraAimTransitionSpeed() const { return WeaponConfig.CameraAimTransitionSpeed; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE FName GetFiringSocket() const { return WeaponSocketing.DefaultFireSocket; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FName GetCameraAimSocket() const;

	/**Returns transform of the camera socket.
	The same result might be achieved by location only.*/
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FTransform GetCameraAimSocketTransform() const;

#pragma endregion public

protected:
#pragma region protected

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	void RefreshOwnerUI();

	UFUNCTION()
	void OnRep_WeaponOwner();
	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	float PlayWeaponAnimation(UAnimMontage* Animation, float InPlayRate = 1.f, FName StartSectionName = NAME_Name);
	UFUNCTION()
	void StopWeaponAnim(UAnimMontage* AnimToPlay);

	void StartFire();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFire();

	void FireWeapon();
	FHitResult WeaponTrace(const FVector &TraceStart, const FVector &TraceEnd) const;
	FVector GetTraceLoc() const;
	FVector GetTraceDir() const;
	void HandleHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir);
	bool CanHit(const FHitResult& Impact);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerHandleHit(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir);

	void StopFire();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire();

	int ConsumeAmmo();

	void OnBurstStarted();
	void OnBurstFinished();

	void HandleFiring();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHandleFiring();
	
	UFUNCTION()
	void OnRep_Reload();
	void StartReload(bool bFromReplication = false);
	void StopReload();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartReload();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopReload();
	void StopSimulateReload();
	void ReloadWeapon();

	void DetermineWeaponState();
	void SetWeaponState(EWeaponState NewWeapoState);

	void OnEquip(EWeaponType Category);
	void OnEquipFinished();
	void OnUnEquip();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnUnEquip();

	void AttachToPawn(EWeaponType Category);
	void DetachFromPawm();

#pragma endregion protected

protected:
#pragma region protected_variables

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponMesh")
	USkeletalMeshComponent* WeaponMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponCollision")
	UBoxComponent* BoxComp = nullptr;

	UPROPERTY(VisibleAnywhere)
	EWeaponState CurrentWeaponState;

	UPROPERTY(EditDefaultsOnly)
	EWeaponType CurrentWeaponCategory;

	UPROPERTY(EditDefaultsOnly)
	EWeaponClassification WeaponClassification;

	// Default weapon values
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FWeaponConfig WeaponConfig;

	// Default weapon socketing according to owning player
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FWeaponSocket WeaponSocketing;

	// Default weapon amoo setting
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FWeaponAmmo WeaponAmmoSettings;

	// Default weapon animations
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FWeaponAnim WeaponAnim;

	// Default weapon attachments
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FWeaponAttachments WeaponAttachments;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_WeaponOwner)
	ACharacter* WeaponOwner = nullptr;

	/**How much ammo is loaded in clip.*/
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 AmmoInClip;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 AmmoInInventory;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;
	bool bIsEquipped;
	bool bPendingEquip;
	bool bWantsToFire;
	bool bRefiring;

	/**last time when this weapon was switched to */
	float EquipStartedTime;
	/**how much time weapon needs to be equipped */
	float EquipDuration;
	/**last time weapon fired*/
	float LastFireTime;

	FTimerHandle TimerHandle_EquipFinishedWeapon;
	FTimerHandle TimerHandle_ReloadWeapon;
	FTimerHandle TimerHandle_StopReload;
	FTimerHandle TimerHandler_FireWeapon;
	FTimerHandle TimerHandle_HandleFiring;

	float TimeBetweenShots;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;
#pragma endregion protected_variables
};
