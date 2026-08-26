// Copyright DevS


#include "UI/ReserveAmmo.h"

#include "Character/HeroCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Weapon/Weapon.h"

void UReserveAmmo::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// 실제 Ammo 데이터를 받기 전 Placeholder UI를 숨긴다.
	Image_WeaponIcon->SetRenderOpacity(0.f);
	Text_Ammo->SetRenderOpacity(0.f);
	
	// Respawn / Re-Possession에 대응하기 위해 PlayerController의 Pawn 변경 Event에 연결한다.
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(
		this,
		&ThisClass::OnPossessedPawnChanged);

	AHeroCharacter* HeroCharacter = Cast<AHeroCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(HeroCharacter))
	{
		return;
	}
	
	// Widget보다 Pawn이 먼저 준비된 경우 현재 Pawn을 즉시 Setup한다.
	OnPossessedPawnChanged(nullptr, HeroCharacter);
	
	// Weapon 상태 초기화
	// 지금 준비되어 있으면 즉시 사용하고, 아니면 준비 완료 Event를 기다린다.
	if (HeroCharacter->HasWeaponFirstReplicated())
	{
		// 인터페이스를 통한 느슨한 의존성 접근
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(HeroCharacter);
		if (IsValid(Weapon))
		{
			// 과거 Delegate Broadcast를 놓쳤더라도 현재 상태를 직접 Pull해서 초기 텍스트를 만든다.
			OnCurrentReserveAmmoChanged(
				IPlayerInterface::Execute_GetReserveAmmo(HeroCharacter),
				Weapon->Ammo, Weapon->WeaponIcon);
		}
	}	
	else	
	{
		// Weapon이 아직 준비되지 않았다면 Replication 완료 Event를 기다린다.
		HeroCharacter->OnWeaponFirstReplicated.AddDynamic(
			this,
			&ThisClass::OnWeaponFirstReplicated);
	}	
	
	if (HeroCharacter->HasAuthority())
	{
		// Authority는 자신의 Weapon을 RepNotify로 다시 받을 필요가 없으므로 현재 상태를 직접 조회한다.
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(HeroCharacter);
		if (!IsValid(Weapon)) return;
		
		// Listen Host / Standalone에서도 현재 Magazine + Reserve Snapshot으로 HUD를 즉시 초기화한다.
		OnCurrentReserveAmmoChanged(
			IPlayerInterface::Execute_GetReserveAmmo(HeroCharacter), 
			Weapon->Ammo, Weapon->WeaponIcon);
	}
}

void UReserveAmmo::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UCombatComponent* OldPawnCombat = UCombatComponent::FindCombatComponent(OldPawn);
	if (IsValid(OldPawnCombat))
	{
		// 이전 Pawn이 더 이상 HUD를 변경하지 못하도록 Event 연결을 해제한다.
		OldPawnCombat->OnCurrentReserveAmmoChanged.RemoveDynamic(
			this,
			&ThisClass::OnCurrentReserveAmmoChanged);

		OldPawnCombat->OnRoundFired.RemoveDynamic(
			this,
			&ThisClass::OnRoundFired);
	}
	
	UCombatComponent* NewPawnCombat = UCombatComponent::FindCombatComponent(NewPawn);

	if (IsValid(NewPawnCombat))
	{
		// 새 Pawn의 Combat Event를 받을 준비가 되었으므로 Widget을 표시한다.
		Image_WeaponIcon->SetRenderOpacity(1.f);
		Text_Ammo->SetRenderOpacity(1.f);

		NewPawnCombat->OnCurrentReserveAmmoChanged.AddDynamic(
			this,
			&ThisClass::OnCurrentReserveAmmoChanged);

		NewPawnCombat->OnRoundFired.AddDynamic(
			this,
			&ThisClass::OnRoundFired);
	}

}

void UReserveAmmo::OnCurrentReserveAmmoChanged(int32 RoundsInReserve, int32 RoundsInWeapon, UMaterialInterface* WeaponIconMaterial)
{
	
	// Weapon이 바뀌면 아이콘도 함께 교체한다.
	if (IsValid(WeaponIconMaterial))
	{
		// UMG Image가 그릴 수 있도록 Material을 Slate Brush Resource로 설정한다.
		FSlateBrush Brush;
		Brush.SetResourceObject(WeaponIconMaterial);
		
		if (IsValid(Image_WeaponIcon))
		{
			// 현재 Weapon의 Icon Material을 실제 Image Widget에 적용한다.
			Image_WeaponIcon->SetBrush(Brush);
		}
	}
	
	if (!IsValid(Text_Ammo))
	{
		return;
	}

	// 사용자에게 보이는 Text는 FText + Localization-friendly Format으로 생성한다.
	const FText AmmoText = FText::Format(
		NSLOCTEXT("AmmoText", "AmmoKey", "{0}/{1}"),
		RoundsInWeapon,
		RoundsInReserve);

	Text_Ammo->SetText(AmmoText);
}

void UReserveAmmo::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve)
{
	// RoundsMax는 Delegate 계약상 전달되지만 이 Widget의 "Current/Reserve" 표시에선 사용하지 않는다.

	if (!IsValid(Text_Ammo))
	{
		return;
	}

	const FText AmmoText = FText::Format(
		NSLOCTEXT("AmmoText", "AmmoKey", "{0}/{1}"),
		RoundsCurrent,
		RoundsInReserve);

	Text_Ammo->SetText(AmmoText);
}

void UReserveAmmo::OnWeaponFirstReplicated(AWeapon* Weapon, bool bTargetingPlayer)
{
	// bTargetingPlayer는 Delegate Signature를 맞추기 위한 값이며 이 Widget에서는 사용하지 않는다.

	AHeroCharacter* HeroCharacter = Cast<AHeroCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(HeroCharacter) || !IsValid(Weapon))
	{
		return;
	}

	// Weapon과 Reserve가 모두 접근 가능한 시점에 현재 Snapshot으로 HUD를 초기화한다.
	OnCurrentReserveAmmoChanged(
		IPlayerInterface::Execute_GetReserveAmmo(HeroCharacter),
		Weapon->Ammo, Weapon->WeaponIcon);
}
