// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HeroGameStateBase.generated.h"

class AHeroPlayerState;
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
	// 단독 리더
	AHeroPlayerState* GetSoleLeader() const;
	// 특정 playerstate가 공동 선두에 포함되어 있는지 확인하는 함수 
	bool IsTiedForTheLead(AHeroPlayerState* PlayerState);

private:
	
	bool bHasFirstBloodBeenHad;
	
	UPROPERTY()
	TArray<TObjectPtr<AHeroPlayerState>> Leaders;
};
