// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReserveAmmo.generated.h"


class AWeapon;
class UTextBlock;
class UImage;

UCLASS()
class FPS_API UReserveAmmo : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeOnInitialized() override;
	
	// 현재 Weapon 아이콘을 표시할 Image.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_WeaponIcon;
	
	// Magazine / Reserve 숫자를 표시할 Text.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Ammo;
	
private:
	// PlayerController가 다른 Pawn을 소유하면 이전 Delegate를 해제하고 새 Pawn에 연결한다.
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	// Reserve Ammo 또는 Weapon Equip 상태가 바뀔 때 전체 Ammo 텍스트를 갱신한다.
	UFUNCTION()
	void OnCurrentReserveAmmoChanged(int32 RoundsInReserve, int32 RoundsInWeapon, UMaterialInterface* WeaponIconMaterial);
	
	// 발사 직후 감소한 Magazine Ammo와 현재 Reserve를 표시한다.
	UFUNCTION()
	void OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve);
	
	// Client에서 Weapon이 늦게 Replicate된 경우 초기 Ammo Snapshot을 다시 가져온다.
	UFUNCTION()
	void OnWeaponFirstReplicated(AWeapon* Weapon, bool bTargetingPlayer);
	
};
