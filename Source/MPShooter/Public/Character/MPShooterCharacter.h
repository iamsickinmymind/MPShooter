// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Weapon.h"
#include "MPShooterCharacter.generated.h"

class AWeapon;

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

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(AWeapon* NewWeapon, EWeaponType Category = EWeaponType::EWT_Primary);

	UFUNCTION(BlueprintPure, Category = "Player")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	bool IsSprinting() const;

#pragma endregion public

protected:
#pragma region protected

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void SetSprinting(bool NewIsSprinting);

	UFUNCTION()
	void OnRep_Weapon();
	void SetActiveWeapon(AWeapon*NewWeapon, AWeapon* LastWeapon = nullptr);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(AWeapon* NewWeapon, EWeaponType Category);

	void SwitchWeapon();

	void OnStartFire();
	void StartFire();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFire();

	void StartReload();

#pragma endregion protected

protected:
#pragma region protected_variables

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeapon>DefaultPrimaryWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeapon>DefaultSecondaryWeapon;

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
#pragma endregion private
};

