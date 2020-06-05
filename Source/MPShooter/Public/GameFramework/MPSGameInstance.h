// All rights reserver Dominik Pavlicek

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MPSGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MPSHOOTER_API UMPSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void HostServer();

	UFUNCTION(BlueprintCallable)
	void FindServer();

	UFUNCTION(BlueprintCallable)
	void JoinServer();

	UFUNCTION(BlueprintCallable)
	void CancelSearch();

protected:

	virtual void Init() override;

	void OnSessionCreated(FName SessionName, bool Success);
	void OnSessionDestroyed(FName SessionName, bool Success);
	void OnFindSessionCompleted(bool Success);
	void OnJoinedSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinSessionResult);

	void CreateSession();
	void StartSession();
	void FindSession();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString GameLevel = "/Game/6_Maps/ExampleMap?listen?game=DefaultGM";
	FString DesiredServerName;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr < FOnlineSessionSearch > SessionSearch;
	int32 TargetSessionIndex;
};
