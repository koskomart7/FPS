// Copyright DevS


#include "Game/HeroGameStateBase.h"
#include "Player/HeroPlayerState.h"

AHeroGameStateBase::AHeroGameStateBase()
{
	bHasFirstBloodBeenHad = false;
}

bool AHeroGameStateBase::HasFirstBloodBeenHad() const
{
	return bHasFirstBloodBeenHad;
}

void AHeroGameStateBase::UpdateLeader()
{
	// GameState의 전체 Player 목록을 로컬 배열로 복사한다.
	TArray<APlayerState*> LocalSortedPlayers = PlayerArray;
	
	// Elimination 수가 높은 Player 부터 내림차순으로 정렬한다.
	LocalSortedPlayers.Sort([](const APlayerState& A, const APlayerState& B)
	{
		const AHeroPlayerState* PlayerA = Cast<AHeroPlayerState>(&A);
		const AHeroPlayerState* PlayerB = Cast<AHeroPlayerState>(&B);
		return PlayerA->GetScoredElims() > PlayerB->GetScoredElims();
	});
	// 이전 Leader 결과를 모두 제거하고 현재 점수 기준으로 다시 계산한다.
	Leaders.Empty();
	
	// 최고 점수 Player와 같은 점수의 모든 Player를 Leaders에 넣는다.
	if (LocalSortedPlayers.Num() > 0)
	{
		int32 HighestScore = 0;
		for (APlayerState* Player : LocalSortedPlayers)
		{
			AHeroPlayerState* HeroPS = Cast<AHeroPlayerState>(Player);
			if (IsValid(HeroPS))
			{
				int32 PlayerScore = HeroPS->GetScoredElims();
				
				// 첫 번째 반복에서 최고 점수를 설정합니다
				if (Leaders.Num() == 0)
				{
					HighestScore = PlayerScore;
					Leaders.Add(HeroPS);
				}
				else if (PlayerScore == HighestScore)
				{
					// 점수가 동률일 경우 리더보드에 추가
					Leaders.Add(HeroPS);
				}
				else
				{
					// 정렬되어 있으므로 더 이상 확인할 필요가 없습니다
					break;	
				}
				
			}
		}
	}
	
	// 누군가 Leader가 생긴 시점이면 First Blood도 이미 발생한 것으로 간주한다.
	bHasFirstBloodBeenHad = true;
}

AHeroPlayerState* AHeroGameStateBase::GetSoleLeader() const
{
	if (Leaders.Num() == 1)
	{
		// 공동 선두가 없는 경우에만 단독 Leader를 반환한다.
		return Leaders[0];
	}
	return nullptr;
}

bool AHeroGameStateBase::IsTiedForTheLead(AHeroPlayerState* PlayerState)
{
	return Leaders.Contains(PlayerState);
}
