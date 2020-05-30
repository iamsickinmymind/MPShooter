// All rights reserved Dominik Pavlicek

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPSHOOTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UHealthComponent();

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetHealth() const { return Health; };

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; };

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE bool IsAlive() const { return bIsAlive; };

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealth(const float NewHealth);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ModifyHealth(const float Value);

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetHealth(const float NewHealth);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServervModifyHealth(const float Value);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (UIMin = 0, ClampMin = 0))
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float Health;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool bIsAlive;
};
