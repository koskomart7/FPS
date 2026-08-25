// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

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
	
public:
	// 각 메쉬에 대한 getter 함수 
	USkeletalMeshComponent* GetMesh1P() const;
	USkeletalMeshComponent* GetMesh3P() const;
};
