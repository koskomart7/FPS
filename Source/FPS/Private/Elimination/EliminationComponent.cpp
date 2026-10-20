// Copyright DevS


#include "Elimination/EliminationComponent.h"

#include "Game/HeroGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HeroPlayerState.h"
#include "ShooterTypes/ShooterTypes.h"


// Sets default values for this component's properties
UEliminationComponent::UEliminationComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;

	// 직전 Kill 이후 이 시간 안에 다음 Kill이 발생하면 같은 Sequence로 본다.
	SequentialElimInterval = 2.f;
	LastElimTime = 0.f;
	SequentialElims = 0;
	// 죽지 않고 이 횟수만큼 kill 하면 streak 상태로 본다
	Streak = 0;
	ElimsNeededForStreak = 5;
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
	ProcessHeadShot(bHeadShot, SpecialElimType, AttackerPS);
	ProcessSequentialEliminations(SpecialElimType, AttackerPS);
	// 이번 Kill을 기준으로 Streak, Showstopper, Revenge까지 함께 판정한다.
	ProcessStreaks(SpecialElimType, AttackerPS, VictimPS);
	
	//UGameplayStatics::GetGameState가 올바른 월드의 게임 상태를 찾을 수 있도록 "현재 게임 월드와 연결된 나침반" 역할로 AttackerPS를 사용
	AHeroGameStateBase* GameState = Cast<AHeroGameStateBase>(UGameplayStatics::GetGameState(AttackerPS));
	if (IsValid(GameState))
	{
		// 먼저 이전 Match 상태를 기준으로 First Blood인지 판정한다.
		HandleFirstBlood(GameState, SpecialElimType, AttackerPS);
		// 그 다음 이번 Kill이 반영된 Leader 상태를 계산한다.
		UpdateLeaderStatus(GameState,SpecialElimType, AttackerPS, VictimPS);
	}
	
	// if( Has Special Elim Types )
		// Tell the client which special elims we got
	// else ( We just got a regular elim )
		// Just tell the client we got a regulat elim
}

void UEliminationComponent::ProcessHeadShot(bool bHeadShot, ESpecialElimType& OutElimType, AHeroPlayerState* AttackerPS)
{
	if (bHeadShot)
	{
		OutElimType |= ESpecialElimType::Headshot;
		AttackerPS->AddHeadShotElim();
	}
}

void UEliminationComponent::ProcessSequentialEliminations(ESpecialElimType& OutElimType, AHeroPlayerState* AttackerPS)
{
	// 현재 서버시간을 얻음
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	// 경과시간이 <= SequentialElimInterval 이라면 
	if (CurrentTime - LastElimTime <= SequentialElimInterval)
	{
		++SequentialElims;
	}
	else
	{
		SequentialElims = 1;
	}
	// Kill 후 반드시 LastElimTime 갱신
	LastElimTime = CurrentTime;
	
	if (SequentialElims > 1)
	{
		OutElimType |= ESpecialElimType::Sequential;
		AttackerPS->AddSequentialElim(SequentialElims);
	}
}

void UEliminationComponent::ProcessStreaks(ESpecialElimType& OutElimType, AHeroPlayerState* AttackerPS,
	AHeroPlayerState* VictimPS)
{
	++Streak;
	if (Streak >= ElimsNeededForStreak)
	{
		OutElimType |= ESpecialElimType::Streak;
		AttackerPS->SetOnStreak(true);
		AttackerPS->UpdateHighestStreak(Streak);
	}
	if (VictimPS->IsOnStreak())
	{
		// Streak 중인 Player를 제거했으므로 Showstopper Event를 추가한다.
		OutElimType |= ESpecialElimType::Showstopper;
		// 공격자의 Match 누적 Showstopper 기록을 증가시킨다.
		AttackerPS->AddShowStopperElim();
		// Victim의 Streak 상태는 이번 Death로 종료된다.
		VictimPS->SetOnStreak(false);
	}
	if (AttackerPS->GetLastAttacker() == VictimPS)
	{
		OutElimType |= ESpecialElimType::Revenge;
		AttackerPS->AddRevengeElim();
		// Revenge를 달성했으므로 현재 Revenge Target을 소모한다.
		AttackerPS->SetLastAttacker(nullptr);
	}
	// Victim의 LastAttacker는 항상 갱신
	VictimPS->SetLastAttacker(AttackerPS);
}

void UEliminationComponent::HandleFirstBlood(AHeroGameStateBase* GameState, ESpecialElimType& OutElimType,
	AHeroPlayerState* AttackerPS)
{
	if (!GameState->HasFirstBloodBeenHad())
	{
		// Match 최초 Kill이면 이번 Elimination에 First Blood Flag를 추가한다.
		OutElimType |= ESpecialElimType::FirstBlood;
		// Attacker PlayerState에도 First Blood 달성 기록을 남긴다.
		AttackerPS->GotFirstBlood();
	}
}

void UEliminationComponent::UpdateLeaderStatus(AHeroGameStateBase* GameState, ESpecialElimType& OutElimType,
	AHeroPlayerState* AttackerPS, AHeroPlayerState* VictimPS)
{
	// 이번 Kill의 Score 변화가 반영되기 전 단독 Leader가 누구였는가?
	AHeroPlayerState* LastLeader = GameState->GetSoleLeader();
	// Attacker가 이전 Leaders 배열에 있었는가?
	const bool bAttackerWasTiedForTheLead = GameState->IsTiedForTheLead(AttackerPS);
	// 이번 Kill Score가 반영된 후의 Leader 재계산
	GameState->UpdateLeader();
	if (!bAttackerWasTiedForTheLead && GameState->IsTiedForTheLead(AttackerPS))
	{
		// 공격자는 이전에는 공동 선두가 아니었으나, 현재는 공동 선두가 되었다
		OutElimType |= ESpecialElimType::TiedTheLeader;
	}
	if (IsValid(LastLeader) && LastLeader != GameState->GetSoleLeader())
	{
		// LastLeader가 선두 자리를 내주었습니다
		LastLeader->Client_LostTheLeader();
		
		if (VictimPS == LastLeader)
		{
			// 이번 Kill의 Victim이 직전 단독 Leader였으므로 Dethrone을 획득한다.
			OutElimType |= ESpecialElimType::Dethrone;
			AttackerPS->AddDethroneElim();
		}
	}
	
	if (AttackerPS != LastLeader && AttackerPS == GameState->GetSoleLeader())
	{
		// 이전 단독 Leader가 아니었던 Attacker가 현재 단독 Leader가 됐다.
		OutElimType |= ESpecialElimType::GainedTheLead;
	}
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


