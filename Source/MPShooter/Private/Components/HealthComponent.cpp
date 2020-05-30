// All rights reserved Dominik Pavlicek

#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);

	MaxHealth = 100.f;
	Health = MaxHealth;
	bIsAlive = true;
}

void UHealthComponent::SetHealth(const float NewHealth)
{
	if (GetOwner())
	{
		if (GetOwnerRole() < ROLE_Authority)
		{
			ServerSetHealth(NewHealth);
		}
		else
		{
			if (NewHealth != Health)
			{
				Health = FMath::Max(NewHealth, 0.f);
				OnRep_Health();
			}
		}
	}
}

void UHealthComponent::ModifyHealth(const float Value)
{
	if (GetOwner())
	{
		if (GetOwnerRole() < ROLE_Authority)
		{
			ServervModifyHealth(Value);
		}
		else
		{
			Health = FMath::Clamp(Health + Value, 0.f, MaxHealth);
			OnRep_Health();
		}
	}
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHealthComponent::OnRep_Health()
{
	if (Health <= 0)
	{
		bIsAlive = false;
	}
}

void UHealthComponent::ServervModifyHealth_Implementation(const float Value)
{
	ModifyHealth(Value);
}

bool UHealthComponent::ServervModifyHealth_Validate(const float Value)
{
	return true;
}

void UHealthComponent::ServerSetHealth_Implementation(const float NewHealth)
{
	SetHealth(NewHealth);
}

bool UHealthComponent::ServerSetHealth_Validate(const float NewHealth)
{
	return true;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
	DOREPLIFETIME(UHealthComponent, bIsAlive);
}