#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ChatPawn.generated.h"

UCLASS()
class CHATX_API AChatPawn : public APawn
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
};
