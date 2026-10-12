// Copyright DevS


#include "Elimination/EliminationComponent.h"

#include "Player/HeroPlayerState.h"
#include "ShooterTypes/ShooterTypes.h"


// Sets default values for this component's properties
UEliminationComponent::UEliminationComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;

	
}

void UEliminationComponent::OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal)
{
	// TODO:
	// Attacker / Victim PlayerState를 찾아
	// Hit, Miss, Headshot, Elimination 등의 통계를 갱신한다.
	
	// UE_LOG(LogTemp, Warning, TEXT("Hit: %d, HeadShot: %d, Lethal: %d"), bHit, bHeadShot, bLethal);
	
	AHeroPlayerState* AttackerPS = GetPlayerStateFromActor(Attacker);
	// 공격자의 PlayerState가 없으면 통계를 기록할 주체가 없으므로 종료한다.
	if (!IsValid(AttackerPS)) return;
	
	ProcessHitOrMiss(bHit, AttackerPS);
	// Miss라면 Victim 통계나 Elimination 처리가 필요 없다.
	if (!bHit) return;
	
	AHeroPlayerState* VictimPS = GetPlayerStateFromActor(Victim);
	
	if (!IsValid(VictimPS)) return;
	
	if (bLethal)
	{
		ProcessElimination(bHeadShot, AttackerPS, VictimPS);
	}
}

void UEliminationComponent::ProcessElimination(bool bHeadShot, AHeroPlayerState* AttackerPS, AHeroPlayerState* VictimPS)
{
	// Lethal Hit으로 확정됐으므로 공격자의 누적 Elimination을 증가시킨다.
	AttackerPS->AddScoredElim();
	VictimPS->AddDefeat();
	
	ESpecialElimType SpecialElimType { };
	// Process Head Shot
	// Process Sequential Eliminations
	// Process Streaks
	// Handle First Blood
	// Update Leader Status
	
	// if( Has Special Elim Types )
		// Tell the client which special elims we got
	// else ( We just got a regular elim )
		// Just tell the client we got a regulat elim
}

void UEliminationComponent::ProcessHitOrMiss(bool bHit, AHeroPlayerState* AttackerPS)
{
	// Victim 없이도 계산 가능 
	if (bHit)
	{
		AttackerPS->AddHit();
	}
	else
	{
		AttackerPS->AddMiss();
	}
}



AHeroPlayerState* UEliminationComponent::GetPlayerStateFromActor(AActor* Actor)
{
	// Round Report의 Actor가 Player Pawn인지 확인한다.
	/*Non-Pawn PlayerInterface Actor
	AI/특수 Object가 별도 통계 Owner를 가짐
	Projectile Actor
	Controller Actor*/
	// 위 액터들은 Player state를 얻지 못해 다양한 공격주체를 통계에 포함 시키지 못한다.
	APawn* Pawn = Cast<APawn>(Actor);
	if (IsValid(Pawn))
	{
		// 이 Pawn에 연결된 PlayerState가 우리의 ShooterPlayerState인지 확인해 반환한다.
		return Pawn->GetPlayerState<AHeroPlayerState>();
	}
	return nullptr;
}


