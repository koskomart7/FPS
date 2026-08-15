// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class UWeaponData;
class AWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	// 인벤토리의 다음 무기 사이클
	void Initiate_CycleWeapon();
	void Initiate_FireWeapon_Pressed();
	void Initiate_FireWeapon_Released();
	void Initiate_ReloadWeapon();
	void Initiate_Aim_Pressed();
	void Initiate_Aim_Released();
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapons")
	TObjectPtr<UWeaponData> WeaponData;		
	
	// 인벤토리 생성 및 파괴
	void SpawnInventory();
	void DestroyInventory();
	
protected:
	

private:
	
	// 단일 무기 스폰 + 포인터 반환 ( 클래스 자체를 값으로 다루는 타입 )
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapons")
	TSubclassOf<AWeapon> DefaultWeaponClass;	// 추후 인벤토리가 생기면 배열로 교체 
	
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
	
};
