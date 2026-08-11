// Copyright DevS


#include "HeroCharacter.h"

// Sets default values
AHeroCharacter::AHeroCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;

}


void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

