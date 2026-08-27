// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PlayerInterface.h"
#include "ShooterTypes/ShooterTypes.h"
#include "HeroCharacter.generated.h"

class AWeapon;
class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponFirstReplicated, AWeapon*, Weapon, bool, bTargetingPlayer);

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
	
	// Player State fallback 
	virtual void OnRep_PlayerState() override;
	
	/** 플레이어 인터페이스 구현 **/
#pragma region Player Interface Section

	virtual FName GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponType) const override;
	
	virtual USkeletalMeshComponent* GetMesh3P_Implementation() const override;
	virtual USkeletalMeshComponent* GetMesh1P_Implementation() const override;
	
	virtual void WeaponReplicated_Implementation() override;
	virtual AWeapon* GetCurrentWeapon_Implementation() override;
	
	virtual int32 GetReserveAmmo_Implementation() const override;
	
	virtual void Notify_CycleWeapon_Implementation() override;
	
#pragma endregion 
	
	UFUNCTION(BlueprintCallable)
	FRotator GetFixedAnimRotation() const;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|FABRIK")
	FTransform FABRIK_SocketTransform;
	
	UFUNCTION(BlueprintCallable)
	bool HasCurrentWeapon() const;
	
	// 델리게이트
	UPROPERTY(BlueprintAssignable)
	FWeaponFirstReplicated OnWeaponFirstReplicated;
	
	// 무기가 처음 복제 되었는지 여부에 대한 getter
	bool HasWeaponFirstReplicated() const { return bWeaponFirstReplicated; }
	
protected:
	UPROPERTY(visibleAnywhere, BlueprintReadOnly, Category = "FPS|Combat" )
	TObjectPtr<UCombatComponent> Combat;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Aiming")
	float DefaultFOV;
	
	// 에임 동작은 블루프린트에서 구현
	UFUNCTION(BlueprintImplementableEvent)	
	void OnAim(bool bIsAiming);
	
	// TurnInPlace 관련 변수
	UPROPERTY(BlueprintReadOnly, Category="FPS|TurnInPlace")
	float AO_Yaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|Strafing")
	float MovementOffsetYaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|TurnInPlace")
	ETurningInPlace TurningStatus;

	
	
private:
	
	void Input_CycleWeapon();
	void Input_ReloadWeapon();
	void Input_FireWeapon_Pressed();
	void Input_FireWeapon_Released();
	void Input_Aim_Pressed();
	void Input_Aim_Released();
	
	void CalculateFABRIKSocketTransform();
	void CalculateTurnInPlaceParameters(float DeltaTime);
	void TurnInPlace(float DeltaTime);
	
	// Character 내부 계산값
	FRotator StartingAimRotation;
	float InterpAO_Yaw;
	
	// 무기가 처음 사용되는 경우 클라이언트 추적 목적
	bool bWeaponFirstReplicated;
	
	
	
	// 1인칭 시점 뷰 (arms)
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> CycleWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> FireWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> ReloadWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> AimWeaponAction;
};
