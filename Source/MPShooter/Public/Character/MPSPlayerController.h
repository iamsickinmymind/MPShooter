// All rights reserved Dominik Pavlicek

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPSPlayerController.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ETeamID : uint8
{
	ETI_TeamA	UMETA(DisplayName = "A"),
	ETI_TeamB	UMETA(DisplayName = "B"),

	ETI_Default UMETA(Hidden)
};

UCLASS()
class MPSHOOTER_API AMPSPlayerController : public APlayerController
{
	GENERATED_BODY()

	AMPSPlayerController();

public:

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateInGameUI();

	UFUNCTION(BlueprintCallable, Client, Reliable)
	void ClientUpdateInGameUI();

	UFUNCTION(BlueprintCallable, Category = "Team")
	void InitializeTeam(ETeamID& InTeamID);

	UFUNCTION(BlueprintPure, Category = "Team")
	FORCEINLINE ETeamID GetTeamID() const { return TeamID; };

	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(const ETeamID NewTeamID);

protected:

	virtual void BeginPlay() override;

protected:
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Team")
	ETeamID TeamID;
};
