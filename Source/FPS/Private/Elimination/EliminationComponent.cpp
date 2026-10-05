// Copyright DevS


#include "Elimination/EliminationComponent.h"


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
	
	UE_LOG(LogTemp, Warning, TEXT("Hit: %d, HeadShot: %d, Lethal: %d"), bHit, bHeadShot, bLethal);
}




