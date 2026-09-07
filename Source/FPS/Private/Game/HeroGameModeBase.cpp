// Copyright DevS


#include "Game/HeroGameModeBase.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void AHeroGameModeBase::RequestRespawn(ACharacter* Character, AController* Controller)
{
	// 기존 Pawn과 이를 다시 연결할 Controller가 모두 필요하다.
	if (!IsValid(Character) || !IsValid(Controller)) return;
	
	Character->Reset();
	Character->Destroy();
	
	//  PlayerStart를 매 Respawn마다 World Search
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
	ensure(PlayerStarts.Num() > 0);
	// Random PlayerStart 선택
	int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
	
	// 지정된 Actor 위치에서 Player Pawn Spawn을 시도
	RestartPlayerAtPlayerStart(Controller, PlayerStarts[Selection]);
}
