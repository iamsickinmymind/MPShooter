// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/HealthComponent.h"
#include "Weapon.h"
#include "MPShooterCharacter.generated.h"

class AWeapon;
class AFireWeapon;

UCLASS(config=Game)
class AMPShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
#pragma region public

	AMPShooterCharacter();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE AWeapon* GetActiveWeapon() const { return ActiveWeapon; };

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE	AActor* GetKiller() const { return Killer; };

	UFUNCTION(BlueprintCallable, Category = "Health")
	FORCEINLINE	float GetHealth() const { return HealthComp ? HealthComp->GetHealth() : 0.f; };

	UFUNCTION(BlueprintCallable, Category = "Health")
	FORCEINLINE	float GetMaxHealth() const { return HealthComp ? HealthComp->GetMaxHealth() : 0.f; };

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(AWeapon* NewWeapon, EWeaponType Category = EWeaponType::EWT_Primary);

	UFUNCTION(BlueprintPure, Category = "Player")
	FORCEINLINE	bool IsAlive() const {return HealthComp ? HealthComp->IsAlive() : false;};

	UFUNCTION(BlueprintPure, Category = "Player")
	bool IsSprinting() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	bool IsAiming();

	UFUNCTION(BlueprintPure, Category = "Player")
	bool CanAim();
#pragma endregion public

protected:
#pragma region protected

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount,struct FDamageEvent const & DamageEvent,class AController * EventInstigator, AActor * DamageCauser) override;
	void Die();
	void TryToDie(const float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser);
	void KilledBy(const AActor* DamageCauser, const AController* EventInstigator, struct FDamageEvent const & DamageEvent);
	void KilledSelf(const AActor* DamageCauser, struct FDamageEvent const & DamageEvent);

	UFUNCTION()
	void OnRep_Killer();

	void SetAiminingCamera(float DeltaTime);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Turn(float Value);
	void LookUp(float Value);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void StartSprint();
	void StopSprint();
	void SetSprinting(bool NewSprinting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(bool NewSprinting);

	UFUNCTION()
	void OnRep_Weapon();
	void SetActiveWeapon(AWeapon*NewWeapon, AWeapon* LastWeapon = nullptr);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(AWeapon* NewWeapon, EWeaponType Category);

	void SwitchWeapon();

	void OnStartFire();
	void StartFire();

	void OnStopFire();
	void StopFire();

	void StartReload();

	void StartAiming();
	void StopAiming();
	void SetAiming(const bool NewAiming);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartAiming();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopAiming();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetAiming(const bool NewAiming);

#pragma endregion protected

protected:
#pragma region protected_variables

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* HealthComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AFireWeapon>DefaultPrimaryWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AFireWeapon>DefaultSecondaryWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Weapon, VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
	AWeapon* ActiveWeapon = nullptr;

	UPROPERTY()
	AWeapon* PreviousWeapon = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseLookUpRate;

#pragma endregion protected_variables

private:
#pragma region private

	bool bWantsToFire;

	UPROPERTY(Transient, Replicated)
	bool bIsAiming;

	UPROPERTY(Transient, Replicated)
	bool bIsSprinting;

	float DefaultFOV;
	FVector DefaultCameraLocation;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Killer)
	AActor* Killer = nullptr;
#pragma endregion private
};

