// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeroGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API AHeroGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	void RequestRespawn(ACharacter* Character, AController* Controller);
	
	// virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
