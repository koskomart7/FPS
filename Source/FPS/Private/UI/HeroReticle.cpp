// Copyright DevS


#include "UI/HeroReticle.h"

#include "Character/HeroCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Weapon/Weapon.h"

namespace Ammo
{
	const FName Rounds_Current = FName("Rounds_Current");
	const FName Rounds_Max = FName("Rounds_Max");
}

namespace Reticle
{
	const FName RoundedCornerScale = FName("RoundedCornerScale");
	const FName ShapeCutThickness = FName("ShapeCutThickness");
}


void UHeroReticle::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Image_Reticle->SetRenderOpacity(0.f);
	Image_AmmoCounter->SetRenderOpacity(0.f);
	_BaseCornerScaleFactor_RoundFired = 0.f;
	_BaseShapeCutFactor_RoundFired = 0.f;
	
	// Pawn이 바뀌는 경우 델리게이트 발송
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	
	// 캐릭터 소유자 확인
	AHeroCharacter* HeroCharacter = Cast<AHeroCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(HeroCharacter)) return;
	
	// Pawn과 캐릭터 소유자 확인시 콜백 직접 호출 
	OnPossessedPawnChanged(nullptr, HeroCharacter);
	
	// 지금 준비되어 있으면 즉시 사용하고, 아니면 준비 완료 Event를 기다린다.
	if (HeroCharacter->HasWeaponFirstReplicated())
	{
		// Get Dynamic Material Instances from the Weapon
		// 인터페이스를 통한 느슨한 의존성 접근
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(HeroCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->ReticleParams);
			OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
		}
	}
	else
	{
		HeroCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
	}
	
	if (HeroCharacter->HasAuthority())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(HeroCharacter);
		if (!IsValid(Weapon)) return;
		OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->ReticleParams);
		OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
	}
}

void UHeroReticle::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	_BaseCornerScaleFactor_RoundFired = FMath::FInterpTo(_BaseCornerScaleFactor_RoundFired, 0.f, InDeltaTime, CurrentReticleParams.RoundFiredInterpSpeed);
	_BaseShapeCutFactor_RoundFired = FMath::FInterpTo(_BaseShapeCutFactor_RoundFired, 0.f, InDeltaTime, CurrentReticleParams.RoundFiredInterpSpeed);
	
	BaseCornerScaleFactor = _BaseCornerScaleFactor_RoundFired + .46;
	BaseShapeCutFactor = _BaseShapeCutFactor_RoundFired -0.8;
	
	if (CurrentReticle_DynMatInst.IsValid())
	{
		CurrentReticle_DynMatInst->SetScalarParameterValue(Reticle::RoundedCornerScale, BaseCornerScaleFactor);
		CurrentReticle_DynMatInst->SetScalarParameterValue(Reticle::ShapeCutThickness, BaseShapeCutFactor);
	}
}

void UHeroReticle::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// Unbind from delegates on the Old Pawn's Combat Component
	// Old Pawn의 전투 컴포넌트에서 델리게이트를 해제합니다. 
	UCombatComponent* OldPawnCombat = UCombatComponent::FindCombatComponent(OldPawn);
	if (IsValid(OldPawnCombat))
	{
		OldPawnCombat->OnReticleChanged.RemoveDynamic(this, &ThisClass::OnReticleChanged);
		OldPawnCombat->OnAmmoCounterChanged.RemoveDynamic(this, &ThisClass::OnAmmoCounterChanged);
		OldPawnCombat->OnRoundFired.RemoveDynamic(this, &ThisClass::OnRoundFired);
	}
	
	// Bind to delegates on the New Pawn's Combat Component
	// New Pawn의 전투 컴포넌트에 델리게이트를 바인딩합니다.
	UCombatComponent* NewPawnCombat = UCombatComponent::FindCombatComponent(NewPawn);
	if (IsValid(NewPawnCombat))
	{
		// 초기 몇 프레임 동안 기본 UMG 머터리얼이 보이는 현상 방지
		Image_Reticle->SetRenderOpacity(1.f);
		Image_AmmoCounter->SetRenderOpacity(1.f);
		
		NewPawnCombat->OnReticleChanged.AddDynamic(this, &ThisClass::OnReticleChanged);
		NewPawnCombat->OnAmmoCounterChanged.AddDynamic(this, &ThisClass::OnAmmoCounterChanged);
		NewPawnCombat->OnRoundFired.AddDynamic(this, &ThisClass::OnRoundFired);
	}
}

void UHeroReticle::OnWeaponFirstReplicated(AWeapon* Weapon)
{
	// Get Dynamic Material Instances from the Weapon.
	OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->ReticleParams);
	OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
		
}

void UHeroReticle::OnReticleChanged(UMaterialInstanceDynamic* ReticleDynMatInst, const FReticleParams& ReticleParams)
{
	
	CurrentReticleParams = ReticleParams;
	
	// Set the material on the actual reticle widget to the dyn mat inst
	// 실제 조준선 위젯의 머티리얼을 dyn mat 인스턴스로 설정합니다.
	CurrentReticle_DynMatInst = ReticleDynMatInst;
	
	FSlateBrush Brush;
	Brush.SetResourceObject(ReticleDynMatInst);
	if (IsValid(Image_Reticle))
	{
		Image_Reticle->SetBrush(Brush);
	}
}

void UHeroReticle::OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynMatInst, int32 RoundsCurrent,
	int32 RoundsMax)
{
	// Set the material on the actual ammo counter widget to the dyn mat inst
	// 실제 탄약 카운터 위젯의 머티리얼을 dyn mat 인스턴스로 설정합니다.
	CurrentAmmoCounter_DynMatInst = AmmoCounterDynMatInst;
	
	// 런타임 MID의 ScalarParameter 변경 
	CurrentAmmoCounter_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Current, RoundsCurrent);
	CurrentAmmoCounter_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Max, RoundsMax);
	
	FSlateBrush Brush;
	Brush.SetResourceObject(AmmoCounterDynMatInst);
	if (Image_AmmoCounter)
	{
		Image_AmmoCounter->SetBrush(Brush);
	}
}

void UHeroReticle::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax)
{
	_BaseCornerScaleFactor_RoundFired += CurrentReticleParams.ScaleFactor_RoundFired;
	_BaseShapeCutFactor_RoundFired += CurrentReticleParams.ShapeCutFactor_RoundFired;
	
	if (CurrentAmmoCounter_DynMatInst.IsValid())
	{
		CurrentAmmoCounter_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Current, RoundsCurrent);
		CurrentAmmoCounter_DynMatInst->SetScalarParameterValue(Ammo::Rounds_Max, RoundsMax);
	}
}

