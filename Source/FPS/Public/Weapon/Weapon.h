// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class FPS_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	
	

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category= "FPS|WeaponType")
	FGameplayTag WeaponType;
	
private:
	
	// 1인칭 메쉬
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	// 3인칭 메쉬
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh3P;
	
	
	
public:
	// 각 메쉬에 대한 getter 함수 
	USkeletalMeshComponent* GetMesh1P() const;
	USkeletalMeshComponent* GetMesh3P() const;
};
