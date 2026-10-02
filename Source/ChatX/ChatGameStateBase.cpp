#include "ChatGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ChatPlayerController.h"

void AChatGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (!HasAuthority())
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PlayerController))
		{
			AChatPlayerController* ChatPlayerController = Cast<AChatPlayerController>(PlayerController);
			if (IsValid(ChatPlayerController))
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				ChatPlayerController->PrintChatMessageString(NotificationString);
			}
		}
	}
}