#include "Player/ChatPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ChatGameModeBase.h"
#include "ChatPlayerState.h"
#include "UI/ChatInput.h"
#include "ChatX/ChatX.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

AChatPlayerController::AChatPlayerController()
{
	bReplicates = true;
}

void AChatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass))
	{
		ChatInputWidgetInstance = CreateWidget<UChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance))
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass))
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance))
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}

void AChatPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;
	//PrintChatMessageString(ChatMessageString);

	if (IsLocalController())
	{
		//ServerRPCPrintChatMessageString(InChatMessageString);
		AChatPlayerState* ChatPlayerState = GetPlayerState<AChatPlayerState>();
		if (IsValid(ChatPlayerState))
		{
			//FString CombinedMessageString = ChatPlayerState->PlayerNameString + TEXT(": ") + InChatMessageString;
			FString CombinedMessageString = ChatPlayerState->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

//사용자가 입력한 메시지를 화면에 출력하는 함수.
void AChatPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	//UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);

	FString NetModeString = ChatXFunctionLibrary::GetNetModeString(this);
	FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	ChatXFunctionLibrary::MyPrintString(this, CombinedMessageString, 10.0f);
}

//Client에서 Server에게 메시지를 전달하는 함수.
void AChatPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

//Server에서 모든 Client에게 메시지를 전달하는 함수.
void AChatPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(this);
	if (IsValid(GameModeBase))
	{
		AChatGameModeBase* ChatGameModeBase = Cast<AChatGameModeBase>(GameModeBase);
		if (IsValid(ChatGameModeBase))
		{
			ChatGameModeBase->PrintChatMessageString(this, InChatMessageString);
		}
	}
}

void AChatPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}