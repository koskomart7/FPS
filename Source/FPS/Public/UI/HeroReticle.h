// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "HeroReticle.generated.h"

class AWeapon;
class UImage;
/**
 * 
 */
UCLASS()
class FPS_API UHeroReticle : public UUserWidget
{
	GENERATED_BODY()

public:
	
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	// 조준선
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Reticle;
	
	// 탄약 카운터
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_AmmoCounter;
	

private:
	// Weapon이 실제 오브젝트 소유자, Widget은 참조 소비자 - 따라서 Widget Reference 때문에 Material Instance의 수명을 강제로 늘리지 않는 것
	// Weapon 교체/ 파괴 이후에도 오래된 MID를 강한 참조로 붙잡는 위험을 줄임 
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentReticle_DynMatInst;
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentAmmoCounter_DynMatInst;

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnWeaponFirstReplicated(AWeapon* Weapon);
	
	UFUNCTION()
	void OnReticleChanged(UMaterialInstanceDynamic* ReticleDynMatInst);
	
	UFUNCTION()
	void OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynMatInst, int32 RoundsCurrent, int32 RoundsMax);
};
