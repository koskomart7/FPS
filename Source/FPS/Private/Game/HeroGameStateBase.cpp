// Copyright DevS


#include "Game/HeroGameStateBase.h"

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
	// 누군가 Leader가 생긴 시점이면 First Blood도 이미 발생한 것으로 간주한다.
	bHasFirstBloodBeenHad = true;
}
