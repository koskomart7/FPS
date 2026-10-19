// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HeroGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API AHeroGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AHeroGameStateBase();
	
	bool HasFirstBloodBeenHad() const;
	void UpdateLeader();

private:
	
	bool bHasFirstBloodBeenHad;
};
