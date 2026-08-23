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
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 인벤토리의 다음 무기 사이클
	void Initiate_CycleWeapon();
	void Initiate_FireWeapon_Pressed();
	void Initiate_FireWeapon_Released();
	void Initiate_ReloadWeapon();
	void Initiate_Aim_Pressed();
	void Initiate_Aim_Released();
	
	// 무기데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly ,Category = "FPS|Weapons")
	TObjectPtr<UWeaponData> WeaponData;		
	
	// 장비 장착 
	void Equip(AWeapon* Weapon);
	
	// 인벤토리 생성 및 파괴
	void SpawnInventory();
	void DestroyInventory();
	
	// 에임 변수 
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bAiming;
	
	// 현재 무기 , 복제 + 클라 도착 시 콜백
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentWeapon)
	TObjectPtr<AWeapon> CurrentWeapon; 
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapon")
	float TraceLength;

private:
	
	// 마지막 무기를 숨기고 비활성화 할 때 강력 ( 교체관련 )
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);	// 파라미터 = 이전값 , 복제 직전의 옛 값이 전달
	
	// 인벤토리 배열 선언
	// 무기 포인터를 디스크에 저장할 수 없다 ( 런타임에만 의미 있는 상태 )
	UPROPERTY(Transient, Replicated)
	TArray<AWeapon*> Inventory;
	
	// 단일 무기 스폰 + 포인터 반환 ( 클래스 자체를 값으로 다루는 타입 ) -> 무기 배열로 교체완료 
	UPROPERTY(EditDefaultsOnly, Category = "FPS|Weapons")
	TArray<TSubclassOf<AWeapon>> DefaultWeaponClasses;	// 추후 인벤토리가 생기면 배열로 교체 - 완료 
	
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
	
	// 에임 관련 함수 ( 서버 RPC )
	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon();
	
	// Local 함수 
	void Local_Aim(bool bPressed);
	void Local_FireWeapon();
};
