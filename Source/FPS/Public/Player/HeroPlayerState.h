// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HeroPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API AHeroPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AHeroPlayerState();
	
	void AddScoredElim();
	void AddDefeat();
	void AddHit();
	void AddMiss();
	void AddHeadShotElim();
	void AddSequentialElim(int32 SequenceCount);
	void UpdateHighestStreak(int32 StreakCount);
	void AddRevengeElim();
	void AddDethroneElim();
	void AddShowStopperElim();
	void GotFirstBlood();
	void IsNowWinner();
	void SetOnStreak(bool bIsOnStreak);
	void SetLastAttacker(APlayerState* Attacker);
	
	bool IsOnStreak();
	APlayerState* GetLastAttacker() const;
	int32 GetScoredElims() const;
	
private:
	int32 ScoredElims;
	int32 Defeats;
	int32 Hits;
	int32 Misses;
	bool bOnStreak; // 스폰된 이후로 몇 번의 연속킬이 있었나요?
	int32 HeadShotElims;
	TMap<int32, int32> SequentialElims;  // Sequential elims - 짧은 기간 내에 여러 차례의 연속킬이 진행되는 방식
	int32 HighestStreak;
	int32 RevengeElims;
	int32 DethroneElims;
	int32 ShowStopperElims;
	bool bFirstBlood;
	bool bWinner;
	
	// 캐릭터가 죽거나 교체되더라도 player identity와 함께 유지되기에 적합한 playerstate 사용  
	TWeakObjectPtr<APlayerState> LastAttacker;
};
