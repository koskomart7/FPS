// Copyright DevS


#include "Combat/CombatComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponData.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/PlayerInterface.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	
	TraceLength = 20'000;
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
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
	Local_FireWeapon();
}

void UCombatComponent::Local_FireWeapon()
{
	if (!IsValid(CurrentWeapon)) return;
	ensure(IsValid(WeaponData));
	
	UAnimMontage* Montage1P = WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponType).FireMontage;
	USkeletalMeshComponent* Mesh1P = IPlayerInterface::Execute_GetMesh1P(GetOwner());
	if (IsValid(Montage1P) && IsValid(Mesh1P))
	{
		Mesh1P->GetAnimInstance()->Montage_Play(Montage1P);
	}
	
	FHitResult Hit;
	CurrentWeapon->WeaponTrace(Hit, TraceLength);
	
	Server_FireWeapon();
}

void UCombatComponent::Server_FireWeapon_Implementation()
{
	Multicast_FireWeapon();
}

void UCombatComponent::Multicast_FireWeapon_Implementation()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (OwningPawn->IsLocallyControlled())
	{
		// do locally-controlled stuff
	}
	else
	{
		ensure(IsValid(WeaponData));
		
		UAnimMontage* Montage3P = WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->WeaponType).FireMontage;
		USkeletalMeshComponent* Mesh3P = IPlayerInterface::Execute_GetMesh3P(GetOwner());
		if (IsValid(Montage3P) && IsValid(Mesh3P))
		{
			Mesh3P->GetAnimInstance()->Montage_Play(Montage3P);
		}
	}
}


void UCombatComponent::Initiate_FireWeapon_Released()
{
	
}

// Local_FireWeapon
	// play the fire weapon montage for the first-person mesh
	// Server_FireWeapon()

// Server_FireWeapon
	// Multicast_FireWeapon()

// Multicast_FireWeapon()
	// if we are NOT locally controlled
	// play the fire montage on the 3rd person mesh

// ======== 한국어 의사코드 ==============

// Local_FireWeapon
	// 1인칭 메시에 대한 발사 무기 몽타주를 재생합니다.
	// Server_FireWeapon()

// Server_FireWeapon
	// Multicast_FireWeapon()

// Multicast_FireWeapon()
	// 로컬로 제어되지 않는 경우
	// 3인칭 메시에 발사 몽타주를 재생

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





