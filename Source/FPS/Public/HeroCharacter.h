// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeroCharacter.generated.h"

UCLASS()
class FPS_API AHeroCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AHeroCharacter();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
protected:
	
	virtual void BeginPlay() override;

private:
	
	
	

};
