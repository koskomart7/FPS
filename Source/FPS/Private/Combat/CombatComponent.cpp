// Copyright DevS


#include "Combat/CombatComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 변수 복제용 표시	
	DOREPLIFETIME(UCombatComponent, Inventory);
	DOREPLIFETIME(UCombatComponent, CurrentWeapon);
	DOREPLIFETIME_CONDITION(UCombatComponent, bAiming, COND_SkipOwner);	// 조건부 알림으로 서버에 똑같은 값을 보낼 필요가 없다
}


void UCombatComponent::Initiate_CycleWeapon()
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f, 
		FColor::Cyan, 
		TEXT("Initiate_CycleWeapon"),
		false);	
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f, 
		FColor::Cyan, 
		TEXT("Initiate_FireWeapon_Pressed"),
		false);	
}

void UCombatComponent::Initiate_FireWeapon_Released()
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f, 
		FColor::Cyan, 
		TEXT("Initiate_FireWeapon_Released"),
		false);	
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f, 
		FColor::Cyan, 
		TEXT("Initiate_ReloadWeapon"),
		false);	
}

void UCombatComponent::Initiate_Aim_Pressed()
{
	Local_Aim(true);
	Server_Aim(true);
}

void UCombatComponent::Initiate_Aim_Released()
{
	Local_Aim(false);
	Server_Aim(false);
}

void UCombatComponent::Server_Aim_Implementation(bool bPressed)
{
	Local_Aim(bPressed);
}

// Aim 상태 변경 시 실행되는 로컬 처리를 하나의 진입점으로 모으는 것 
void UCombatComponent::Local_Aim(bool bPressed)
{
	bAiming = bPressed;
}

void UCombatComponent::Equip(AWeapon* Weapon)
{
	CurrentWeapon = Weapon;					// 값 변경 = 복제 트리거
	CurrentWeapon->AttachToOwningPawn();	// 서버 몫의 부착 (서버는 OnRep 자동 호출 없음)
}

// 인벤토리 생성
void UCombatComponent::SpawnInventory()
{
	// 권한 점검 - 서버가 아니면 즉시 종료 
	if(GetOwner()->GetLocalRole() < ROLE_Authority) return;

	
	for(TSubclassOf<AWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		AWeapon* Weapon = SpawnWeapon(WeaponClass);
		Inventory.AddUnique(Weapon);	// 동일 포인터 중복만 거름 
	}

	if(Inventory.Num() > 0)		// 빈 배열 인덱싱 방지
	{
		Equip(Inventory[0]);
	}
}

void UCombatComponent::DestroyInventory()
{
	for(AWeapon* Weapon : Inventory)
	{
		if(IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}
}

void UCombatComponent::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	//	게임 중 무기를 바꾸는 상황은 OnRep_CurrentWeapon이 맡는다
	if (!IsValid(CurrentWeapon)) return;

	CurrentWeapon->AttachToOwningPawn();
}


AWeapon* UCombatComponent::SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const
{
	AActor* OwningActor = GetOwner();
	if (!IsValid(OwningActor)) return nullptr;
	if (OwningActor->GetLocalRole() < ROLE_Authority) return nullptr;	// 네트워크 권한 값 비교 
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Cast<APawn>(OwningActor);
	SpawnInfo.Owner = OwningActor;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	return GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnInfo);
	
}



