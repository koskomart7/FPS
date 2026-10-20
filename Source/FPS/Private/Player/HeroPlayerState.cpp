// Copyright DevS


#include "Player/HeroPlayerState.h"

AHeroPlayerState::AHeroPlayerState()
{
	NetUpdateFrequency = 100.f;
	
	ScoredElims = 0;
	Defeats = 0;
	Hits = 0;
	Misses = 0;
	bOnStreak = false;
	HeadShotElims = 0;
	HighestStreak = 0;
	RevengeElims = 0;
	DethroneElims = 0;
	ShowStopperElims = 0;
	bFirstBlood = false;
	bWinner = false;
}

void AHeroPlayerState::AddScoredElim()
{
	++ScoredElims;
}

void AHeroPlayerState::AddDefeat()
{
	++Defeats;
}

void AHeroPlayerState::AddHit()
{
	++Hits;
}

void AHeroPlayerState::AddMiss()
{
	++Misses;
}

void AHeroPlayerState::AddHeadShotElim()
{
	++HeadShotElims;
}

void AHeroPlayerState::AddSequentialElim(int32 SequenceCount)
{
	/*
	Value: Match에서 해당 Sequence를 달성한 횟수
	예:
	{
		2 : 3,   // Double Kill 3회
		3 : 1,   // Triple Kill 1회
		4 : 2    // Quad Kill 2회
	}
	*/
	
	if (SequentialElims.Contains(SequenceCount))
	{
		// 이미 기록이 있으면 해당 Sequence 달성 횟수를 증가시킨다.
		SequentialElims[SequenceCount]++;
	}
	else
	{
		// 처음 달성한 Sequence라면 1회로 등록한다.
		SequentialElims.Add(SequenceCount, 1);
	}
	
	/* Reduce the count for all lower sequence counts
	 * this is because a triple elim means a double was
	 * scored first. But we want to count this as just a triple,
	 * i.e. elim 1, elim 2, elim 3 = just a triple, not a double and a triple
	 
	* 모든 낮은 순서 점수의 합계를 줄이세요.
	 * 이는 트리플 엘리미네이션이 더블이 먼저 기록되었음을 의미하기 때문입니다.
	 * 하지만 우리는 이를 단순히 트리플로만 계산하고자 합니다.
	 * 즉, 엘리미네이션 1, 엘리미네이션 2, 엘리미네이션 3은 단순히 트리플일 뿐, 더블과 트리플이 따로 있는 것이 아닙니다.
	 * Triple이 되면 그 Chain에서 직전에 기록한 Double을 취소	
	 */
	for (auto& Elem : SequentialElims)
	{
		// 현재 Sequence보다 낮은 기록 중 / 양수인 항목을 하나씩 차감한다.
		if (Elem.Key < SequenceCount && Elem.Value > 0)
		{
			Elem.Value--;
		}
	}
	
}

void AHeroPlayerState::UpdateHighestStreak(int32 StreakCount)
{
	if (StreakCount > HighestStreak)
	{
		HighestStreak = StreakCount;
	}
}

void AHeroPlayerState::AddRevengeElim()
{
	++RevengeElims;
}

void AHeroPlayerState::AddDethroneElim()
{
	++DethroneElims;
}

void AHeroPlayerState::AddShowStopperElim()
{
	++ShowStopperElims;
}

void AHeroPlayerState::GotFirstBlood()
{
	bFirstBlood = true;
}

void AHeroPlayerState::IsNowWinner()
{
	bWinner = true;
}

void AHeroPlayerState::SetOnStreak(bool bIsOnStreak)
{
	// Streak 진입/종료 상태를 Server PlayerState에 기록한다.
	bOnStreak = bIsOnStreak;
}

void AHeroPlayerState::SetLastAttacker(APlayerState* Attacker)
{
	// Revenge 판정을 위해 마지막 killer를 약한 참조로 저장한다.
	LastAttacker = Attacker;
}

bool AHeroPlayerState::IsOnStreak()
{
	// 현재 이 Player가 Streak 상태인지 읽는다.
	return bOnStreak;
}

APlayerState* AHeroPlayerState::GetLastAttacker() const
{
	// Weak Pointer 대상이 아직 살아 있을 때만 raw 포인터를 반환한다.
	return LastAttacker.IsValid() ? LastAttacker.Get() : nullptr;
}

int32 AHeroPlayerState::GetScoredElims() const
{
	// Leader 계산에서 사용할 현재 누적 Elimination 수를 반환한다.
	return ScoredElims;
}
