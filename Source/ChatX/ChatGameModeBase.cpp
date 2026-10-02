#include "ChatGameModeBase.h"
#include "ChatGameStateBase.h"
#include "Player/ChatPlayerController.h"
#include "Player/ChatPlayerState.h"
#include "EngineUtils.h"

void AChatGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	AChatPlayerController* ChatPlayerController = Cast<AChatPlayerController>(NewPlayer);
	if (IsValid(ChatPlayerController))
	{
		ChatPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
		AllPlayerControllers.Add(ChatPlayerController);

		AChatPlayerState* ChatPlayerState = ChatPlayerController->GetPlayerState<AChatPlayerState>();
		if (IsValid(ChatPlayerState))
		{
			ChatPlayerState->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		AChatGameStateBase* ChatGameStateBase = GetGameState<AChatGameStateBase>();
		if (IsValid(ChatGameStateBase))
		{
			ChatGameStateBase->MulticastRPCBroadcastLoginMessage(ChatPlayerState->PlayerNameString);
		}
	}
}

FString AChatGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i < 10; ++i)
	{
		Numbers.Add(i);
	}
	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}
bool AChatGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {
		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;

	} while (false);

	return bCanPlay;
}
FString AChatGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void AChatGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Log, TEXT("SecretNumberString: %s"), *SecretNumberString);
}

void AChatGameModeBase::PrintChatMessageString(AChatPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	FString ChatMessageString = InChatMessageString;
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString))
	{
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		IncreaseGuessCount(InChattingPlayerController);

		for (TActorIterator<AChatPlayerController> It(GetWorld()); It; ++It)
		{
			AChatPlayerController* ChatPlayerController = *It;
			if (IsValid(ChatPlayerController))
			{
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				ChatPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);

				int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
				JudgeGame(InChattingPlayerController, StrikeCount);
			}
		}
	}
	else
	{
		for (TActorIterator<AChatPlayerController> It(GetWorld()); It; ++It)
		{
			AChatPlayerController* ChatPlayerController = *It;
			if (IsValid(ChatPlayerController))
			{
				ChatPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
	}
}

void AChatGameModeBase::IncreaseGuessCount(AChatPlayerController* InChattingPlayerController)
{
	AChatPlayerState* ChatPlayerState = InChattingPlayerController->GetPlayerState<AChatPlayerState>();
	if (IsValid(ChatPlayerState))
	{
		ChatPlayerState->CurrentGuessCount++;
	}
}

void AChatGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();

	for (const auto& ChatPlayerController : AllPlayerControllers)
	{
		AChatPlayerState* ChatPlayerState = ChatPlayerController->GetPlayerState<AChatPlayerState>();
		if (IsValid(ChatPlayerState))
		{
			ChatPlayerState->CurrentGuessCount = 0;
		}
	}
}

void AChatGameModeBase::JudgeGame(AChatPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		AChatPlayerState* ChatPlayerState = InChattingPlayerController->GetPlayerState<AChatPlayerState>();
		for (const auto& ChatPlayerController : AllPlayerControllers)
		{
			if (IsValid(ChatPlayerState))
			{
				FString CombinedMessageString = ChatPlayerState->PlayerNameString + TEXT(" has won the game.");
				ChatPlayerController->NotificationText = FText::FromString(CombinedMessageString);

				ResetGame();
			}
		}
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& ChatPlayerController : AllPlayerControllers)
		{
			AChatPlayerState* ChatPlayerState = ChatPlayerController->GetPlayerState<AChatPlayerState>();
			if (IsValid(ChatPlayerState))
			{
				if (ChatPlayerState->CurrentGuessCount < ChatPlayerState->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)
		{
			for (const auto& ChatPlayerController : AllPlayerControllers)
			{
				ChatPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));

				ResetGame();
			}
		}
	}
}