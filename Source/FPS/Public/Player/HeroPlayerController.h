// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HeroPlayerController.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class FPS_API AHeroPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	AHeroPlayerController();
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
private:
	UPROPERTY(EditAnywhere, Category= "FPS|Input")
	TObjectPtr<UInputMappingContext> HeroIMC;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> JumpAction;
	
	void Input_Crouch();
	void Input_Jump();
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
};
