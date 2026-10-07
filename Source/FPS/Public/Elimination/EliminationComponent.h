// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EliminationComponent.generated.h"


class AHeroPlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UEliminationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEliminationComponent();
	
	UFUNCTION()
	void OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal);

private:
	
	// OnRoundReported() 를 거대 함수로 만들기보다 각각 Helper 함수로 분리
	AHeroPlayerState* GetPlayerStateFromActor(AActor* Actor);	// Actor → PlayerState 변환
	void ProcessHitOrMiss(bool bHit, AHeroPlayerState* AttackerPS);		// Hit/Miss 처리
	void ProcessElimination(bool bHeadShot, AHeroPlayerState* AttackerPS, AHeroPlayerState* VictimPS);	// Elimination 처리
};
