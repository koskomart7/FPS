// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EliminationComponent.generated.h"


enum class ESpecialElimType : uint16;
class AHeroPlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UEliminationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEliminationComponent();
	
	UFUNCTION()
	void OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal);

	UPROPERTY(EditDefaultsOnly, Category = "FPS|Elimination")
	float SequentialElimInterval;
	
private:
	
	// 직전 Lethal Elimination이 발생한 서버 World Time
	float LastElimTime;
	// 현재 진행 중인 Kill Chain의 Kill 수
	int32 SequentialElims;
	
	// OnRoundReported() 를 거대 함수로 만들기보다 각각 Helper 함수로 분리
	AHeroPlayerState* GetPlayerStateFromActor(AActor* Actor);	// Actor → PlayerState 변환
	void ProcessHitOrMiss(bool bHit, AHeroPlayerState* AttackerPS);		// Hit/Miss 처리
	void ProcessElimination(bool bHeadShot, AHeroPlayerState* AttackerPS, AHeroPlayerState* VictimPS);	// Elimination 처리
	void ProcessHeadShot(bool bHeadShot, ESpecialElimType& OutElimType, AHeroPlayerState* AttackerPS);
	void ProcessSequentialEliminations(ESpecialElimType& OutElimType, AHeroPlayerState* AttackerPS);
	
};
