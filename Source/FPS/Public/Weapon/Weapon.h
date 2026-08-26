// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ShooterTypes/ShooterTypes.h"
#include "Weapon.generated.h"

class UMaterialInstanceDynamic;
enum EPhysicalSurface : int;

UENUM(BlueprintType)
enum class EFireType : uint8 
{
	Auto UMETA(DisplayName = "Automatic"),
	SemiAuto UMETA(DisplayName = "SemiAutomatic")
};


UCLASS()
class FPS_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	
	virtual void OnRep_Instigator() override; 	
	
	void AttachToOwningPawn() const;
	void WeaponTrace(FHitResult& OutHit, float TraceLenght);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "FPS|WeaponType")
	FGameplayTag WeaponType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Aiming")
	float AimFOV;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Trace")
	float TraceRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|FireType")
	EFireType FireType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|FireType")
	float FireTime;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Reticle")
	FReticleParams ReticleParams;
	
	void Local_Fire(
		const FVector& ImpactPoint, 
		const FVector& ImpactNormal, 
		TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, 
		bool bIsFirstPerson
		);
	
	void Auth_Fire();
	void Rep_Fire(int32 AuthAmmo);
	
	// Weapon Magazine이 담을 수 있는 최대 탄 수
	UPROPERTY(EditAnywhere, Category="FPS|Ammo")
	int32 MagCapacity;
	
	// 현재 이 Weapon Magazine 안에 들어 있는 탄 수
	UPROPERTY(EditAnywhere, Category="FPS|Ammo")
	int32 Ammo;
	
	// Weapon Type을 위해 Character가 처음 보유할 예비 탄약
	UPROPERTY(EditAnywhere, Category="FPS|Ammo")
	int32 StartingCarriedAmmo;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void FireEffects(
		const FVector& ImpactPoint, 
		const FVector& ImpactNormal,
		EPhysicalSurface ImpactSurfaceType,
		bool bIsFirstPerson
		);
	
	// 1인칭 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	// 3인칭 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<USkeletalMeshComponent> Mesh3P;
	
private:
	
	// 메쉬 가시성 설정 함수 
	void SetMeshVisibilities(APawn* OwingPawn) const;
	
	// 아직 Server가 확인하지 않은 Local Ammo 감소 횟수
	int32 Sequence;
	
	// UI 관련 머터리얼
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TObjectPtr<UMaterialInterface> ReticleMaterial;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapon")
	TObjectPtr<UMaterialInterface> AmmoCounterMaterial;
	
	// 런타임 다이나믹 머터리얼 인스턴스
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynMatInst_Reticle;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynMatInst_AmmoCounter;
	
	
public:
	// 각 메쉬에 대한 getter 함수 
	USkeletalMeshComponent* GetMesh1P() const;
	USkeletalMeshComponent* GetMesh3P() const;
	
	// MID getter 함수 
	UMaterialInstanceDynamic* GetReticleDynamicMaterialInstance();
	UMaterialInstanceDynamic* GetAmmoCounterDynamicMaterialInstance();
};
