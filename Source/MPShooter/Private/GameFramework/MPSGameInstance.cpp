// All rights reserver Dominik Pavlicek


#include "MPSGameInstance.h"

const static FName NAME_Server = TEXT("ServerCustomName");

void UMPSGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* ActiveOnlineSubsystem = IOnlineSubsystem::Get();

	if (ActiveOnlineSubsystem)
	{
		SessionInterface = ActiveOnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMPSGameInstance::OnSessionCreated);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMPSGameInstance::OnSessionDestroyed);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMPSGameInstance::OnFindSessionCompleted);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMPSGameInstance::OnJoinedSessionCompleted);
		}
	}
}

void UMPSGameInstance::HostServer()
{
	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (ExistingSession)
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
		else
		{
			CreateSession();
		}
	}
}

void UMPSGameInstance::FindServer()
{
	FindSession();
}

void UMPSGameInstance::JoinServer()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[TargetSessionIndex]);
	}
}

void UMPSGameInstance::CancelSearch()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->CancelFindSessions();
	}
}

void UMPSGameInstance::OnSessionCreated(FName SessionName, bool Success)
{
	if (!Success) return;

	if (GetWorld() != nullptr) 
	{
		GetWorld()->ServerTravel("/Game/6_Maps/ExampleMap?listen?game=DefaultGM");
	}
}

void UMPSGameInstance::OnSessionDestroyed(FName SessionName, bool Success)
{
	if (Success) {

		CreateSession();
	}
}

void UMPSGameInstance::OnFindSessionCompleted(bool Success)
{
	if (!Success)
	{
		if (GetEngine()) GetEngine()->AddOnScreenDebugMessage(0, 5.f, FColor::Red, FString("OnFindSessionCompleted failed 1st Step"));
		return;
	}
// 	Success = (SessionSearch.IsValid());
// 	if (!Success)
// 	{
// 		if (GetEngine()) GetEngine()->AddOnScreenDebugMessage(0, 5.f, FColor::Red, FString("OnFindSessionCompleted failed 2nd Step"));
// 		return;
// 	}
// 	if (SessionSearch->SearchResults.Num() <= 0)
// 	{
// 		if (GetEngine()) GetEngine()->AddOnScreenDebugMessage(0, 5.f, FColor::Red, FString("OnFindSessionCompleted failed 3rd Step"));
// 		return;
// 	}

	if (SessionSearch.IsValid())
	{
		TargetSessionIndex = FMath::RandRange(0, SessionSearch->SearchResults.Num() - 1);
		JoinServer();
	}
}

void UMPSGameInstance::OnJoinedSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinSessionResult)
{
	if (!SessionInterface.IsValid()) return;

	FString ConnectString;

	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString)) 
	{
		GetPrimaryPlayerController()->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
	}
}

void UMPSGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		DesiredServerName = "randomServerName"; //FString::FromInt(FMath::RandRange(100000000000000, 999999999999999));

		FOnlineSessionSettings SessionSettings;

			SessionSettings.NumPublicConnections = 4;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bUsesPresence = true;
			SessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
			SessionSettings.Set(NAME_Server, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
	}
}

void UMPSGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->StartSession(NAME_GameSession);
	}
}

void UMPSGameInstance::FindSession()
{
	if (SessionInterface.IsValid())
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());

		if (SessionSearch.IsValid() && SessionInterface.IsValid()) {

			SessionSearch->TimeoutInSeconds = 30.f;
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 100;
			SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

			SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}
}
