// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PlayerInterface.h"
#include "HeroCharacter.generated.h"

class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;

UCLASS()
class FPS_API AHeroCharacter : public ACharacter, public IPlayerInterface
{
	GENERATED_BODY()

public:
	
	AHeroCharacter();
	
	virtual void BeginPlay() override;
	
	virtual void BeginDestroy() override;

	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	/** 플레이어 인터페이스 구현 **/
#pragma region Player Interface Section

	virtual FName GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponType) const override;
	
	virtual USkeletalMeshComponent* GetMesh3P_Implementation() const override;
	virtual USkeletalMeshComponent* GetMesh1P_Implementation() const override;
	
#pragma endregion 
	
	
protected:
	UPROPERTY(visibleAnywhere, BlueprintReadOnly, Category = "FPS|Combat" )
	TObjectPtr<UCombatComponent> Combat;
	
private:
	
	void Input_CycleWeapon();
	void Input_ReloadWeapon();
	void Input_FireWeapon_Pressed();
	void Input_FireWeapon_Released();
	void Input_Aim_Pressed();
	void Input_Aim_Released();
	
	
	// 1인칭 시점 뷰 (arms)
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> CycleWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> FireWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> ReloadWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> AimWeaponAction;
};
