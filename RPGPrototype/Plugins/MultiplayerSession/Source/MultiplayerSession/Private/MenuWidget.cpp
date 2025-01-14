// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "Components/Button.h"
#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

void UMenuWidget::MenuSetup(int32 NumberOfPublicConnection, FString TypeOfMatch, FString LobbyPath)
{
	NumPublicConnection = NumberOfPublicConnection;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}

	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenuWidget::OnCreateSession);
		MultiplayerSessionSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UMenuWidget::OnFindSessions);
		MultiplayerSessionSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenuWidget::OnJoinSession);
		MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenuWidget::OnDestroySession);
		MultiplayerSessionSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMenuWidget::OnStartSession);
	}
}

bool UMenuWidget::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success) { return false; }

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenuWidget::HostButtonClicked);
	}

	if (JoinButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenuWidget::JoinButtonClicked);
	}

	return true;
}

void UMenuWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenuWidget::HostButtonClicked()
{
	//UE_LOG(LogTemp, Warning, TEXT("Host Button Clicked"));

	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->CreateSession(NumPublicConnection, MatchType);
	}
}

void UMenuWidget::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->FindSession(10000);
	}
}

void UMenuWidget::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UMenuWidget::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("Session Create Successfully")));

		UWorld* World = GetWorld();
		if (World)
		{
			// listen wait other player to join
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		// TODO 
		HostButton->SetIsEnabled(true);
	}
}

void UMenuWidget::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful)
{
	if (!MultiplayerSessionSubsystem)
	{
		return;
	}

	for (auto Result : SessionResult)
	{
		FString SettingValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingValue);
		if (SettingValue == MatchType)
		{
			MultiplayerSessionSubsystem->JoinSession(Result);
		}
	}

	if (!bWasSuccessful || SessionResult.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenuWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSubsytem = IOnlineSubsystem::Get();
	if (OnlineSubsytem)
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsytem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenuWidget::OnDestroySession(bool bWasSuccessful)
{

}

void UMenuWidget::OnStartSession(bool bWasSuccessful)
{

}