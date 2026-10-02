#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ChatGameModeBase.generated.h"

class AChatPlayerController;

UCLASS()
class CHATX_API AChatGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPostLogin(AController* NewPlayer) override;

	FString GenerateSecretNumber();
	bool IsGuessNumberString(const FString& InNumberString);
	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	virtual void BeginPlay() override;

	void PrintChatMessageString(AChatPlayerController* InChattingPlayerController, const FString& InChatMessageString);
	void IncreaseGuessCount(AChatPlayerController* InChattingPlayerController);
	void ResetGame();
	void JudgeGame(AChatPlayerController* InChattingPlayerController, int InStrikeCount);

protected:
	FString SecretNumberString;
	TArray<TObjectPtr<AChatPlayerController>> AllPlayerControllers;
};
