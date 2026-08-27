// Copyright DevS


#include "Combat/CombatComponent.h"

#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponData.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/PlayerInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include "FPS/FPS.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	
	TraceLength = 20'000;
	bAiming = false;
	bTriggerPressed = false;
	
	// 현재 선택된 Inventory 위치를 첫 번째 Weapon으로 초기화한다.
	Local_WeaponIndex = 0;
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwningPawn) || !OwningPawn->IsLocallyControlled()) return;
	
	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!IsValid(PC)) return;
	
	// PlayerController의 Eyes Viewpoint로 조준선 만들기
	FVector EyesWorldLocation;
	FRotator EyesWorldRotation;
	PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
	const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
	
	const FVector Start = EyesWorldLocation;
	const FVector End = Start + EyesWorldDirection * TraceLength;
	
	// Targeting 전용 Collision Filtering - 자기 자신 무시
	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	// Pawn과 PhysicBody 는 Block 나머지는 Ignore
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
	
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, FPSTraceChannels::ECC_Weapon, QueryParams, ResponseParams);
	
	// Hit Actor가 Player인지 PlayerInterface로 판정
	bHitPlayer = IsValid(Hit.GetActor()) && Hit.GetActor()->Implements<UPlayerInterface>();
	
	// 매프레임을 이전 프레임과 비교해 "변화할 때만" 델리게이트 발송
	if (bHitPlayer != bHitPlayerLastFrame)
	{
		OnTargetingPlayerStatusChanged.Broadcast(bHitPlayer);
	}
	
	bHitPlayerLastFrame = bHitPlayer;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 변수 복제용 표시	
	DOREPLIFETIME(UCombatComponent, Inventory);
	DOREPLIFETIME(UCombatComponent, CurrentWeapon);
	DOREPLIFETIME_CONDITION(UCombatComponent, bAiming, COND_SkipOwner);	// 조건부 알림으로 서버에 똑같은 값을 보낼 필요가 없다
	DOREPLIFETIME_CONDITION(UCombatComponent, CurrentReserveAmmo, COND_OwnerOnly);  // 소유자에게만 동기화
}


void UCombatComponent::Initiate_CycleWeapon()
{
	// 현재 장착 Weapon이 없으면 Cycle할 수 없다.
	if (!IsValid(CurrentWeapon)) return;
	// 이미 Weapon Cycle 연출이 진행 중이면 중복 입력을 무시한다.
	if (CurrentWeapon->WeaponStatus == EWeaponStatus::Cycling) return;
	
	// Inventory에서 다음 Weapon Index로 이동한다.
	AdvanceWeaponIndex();
	
	
	// Owner에서 다음 Weapon의 Equip Montage를 즉시 재생한다.
	Local_CycleWeapon(Local_WeaponIndex);
}

// Local_CycleWeapon
	// WeaponIndex에서 해당 무기의 장비 몽타주를 재생합니다.
		// 로컬로 제어되는 경우 1P를 재생하고, 그렇지 않은 경우 3P를 재생합니다.
	// 로컬로 제어되는 경우 Server_CycleWeapon()을 호출합니다.
	// CurrentWeapon의 WeaponStatus를 Cycling으로 설정합니다.

// Server_CycleWeapon
	// 로컬 무기 인덱스 설정
	// Multicast_CycleWeapon(WeaponIndex)

// Multicast_CycleWeapon
	// 로컬 무기 인덱스 설정	
	// Local_CycleWeapon(WeaponIndex)

void UCombatComponent::Local_CycleWeapon(int32 WeaponIndex)
{
	// 전달된 Index의 Weapon을 "교체될 다음 Weapon"으로 사용한다.
	AWeapon* NextWeapon = Inventory[WeaponIndex];

	// 다음 Weapon 또는 WeaponData가 없으면 Montage를 재생할 수 없다.
	if (!IsValid(NextWeapon) || !IsValid(WeaponData))
	{
		return;
	}

	// 현재 Weapon과 다음 Weapon을 모두 Cycle 처리 중으로 잠근다.
	CurrentWeapon->WeaponStatus = EWeaponStatus::Cycling;
	NextWeapon->WeaponStatus = EWeaponStatus::Cycling;

	// Local 여부로 1P / 3P Montage Data 선택
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	const bool bIsLocal = IsValid(OwningPawn) && OwningPawn->IsLocallyControlled();
	
	const FMontageData& MontageData = bIsLocal
	? WeaponData->FirstPersonMontages.FindChecked(NextWeapon->WeaponType)
	: WeaponData->ThirdPersonMontages.FindChecked(NextWeapon->WeaponType);

	// PlayerInterface로 1P / 3P Mesh 선택
	USkeletalMeshComponent* Mesh = bIsLocal
	? IPlayerInterface::Execute_GetMesh1P(GetOwner())
	: IPlayerInterface::Execute_GetMesh3P(GetOwner());
	
	if (IsValid(Mesh) && IsValid(MontageData.EquipMontage))
	{
		// 선택된 Mesh의 AnimInstance에서 다음 Weapon용 Equip Montage를 재생한다.
		Mesh->GetAnimInstance()->Montage_Play(MontageData.EquipMontage);
	}
	
	if (bIsLocal)
	{
		// 입력을 실제로 소유한 Local Pawn만 Server에 Cycle 요청을 전송한다.
		Server_CycleWeapon(WeaponIndex);
		
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();

		if (IsValid(AnimInstance))
		{
			// Owner의 1P Equip Montage가 Blend Out을 시작하면 Cycle 종료 Callback을 실행한다.
			AnimInstance->OnMontageBlendingOut.AddDynamic(
				this,
				&ThisClass::BlendOut_CycleWeapon);
		}
	}

}


void UCombatComponent::Server_CycleWeapon_Implementation(int32 WeaponIndex)
{
	// Owner가 선택한 다음 Weapon Index를 Server 쪽 Local State에도 맞춘다.
	Local_WeaponIndex = WeaponIndex;

	// Server와 현재 연결된 Client에 Cycle 연출을 전파한다.
	Multicast_CycleWeapon(WeaponIndex);
	
}

void UCombatComponent::Multicast_CycleWeapon_Implementation(int32 WeaponIndex)
{
	// Owner 중복 실행 방지
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());

	if (!IsValid(OwningPawn))
	{
		return;
	}

	// Owner는 이미 Local Prediction으로 1P Montage를 재생했으므로 중복 실행하지 않는다.
	if (!OwningPawn->IsLocallyControlled())
	{
		Local_WeaponIndex = WeaponIndex;
		Local_CycleWeapon(WeaponIndex);
	}
}

void UCombatComponent::Notify_CycleWeapon()
{
	// 현재 Weapon이 없으면 Notify가 와도 교체할 수 없다.
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	// Initiate 단계에서 미리 선택해 둔 Inventory Index의 Weapon을 가져온다.
	AWeapon* NewWeapon = Inventory[Local_WeaponIndex];

	if (IsValid(NewWeapon))
	{
		// Montage 중간의 숨겨진 시점에서 실제 Equip 절차를 시작한다.
		EquipWeapon(NewWeapon);
	}
}

void UCombatComponent::BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	// Owner의 1P AnimInstance를 가져온다.
	USkeletalMeshComponent* Mesh1P = IPlayerInterface::Execute_GetMesh1P(GetOwner());

	if (!IsValid(Mesh1P))
	{
		return;
	}

	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();

	if (!IsValid(AnimInstance))
	{
		return;
	}

	// 다음 Cycle에서 동일 Callback이 중복 등록되지 않도록 현재 Binding을 제거한다.
	if (AnimInstance->OnMontageBlendingOut.IsAlreadyBound(this, &ThisClass::BlendOut_CycleWeapon))
	{
		AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &ThisClass::BlendOut_CycleWeapon);
	}
	
	// 현재 강의에서는 Cycle 완료 테스트를 위해 CurrentWeapon을 Idle로 복구한다.
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->WeaponStatus = EWeaponStatus::Idle;
	}
	
	// Montage Notify가 원하는 타이밍에 정상 호출되는지 먼저 검증한다.
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Yellow,
			TEXT("Blend Out Cycle Weapon"),
			false);
	}
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	if (!IsValid(CurrentWeapon)) return;
	
	bTriggerPressed = true;
	
	if (CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
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
	
	EPhysicalSurface ImpactSurfaceType = 
		Hit.PhysMaterial.IsValid(false) 
	? Hit.PhysMaterial->SurfaceType.GetValue() 
	: SurfaceType1;
	
	CurrentWeapon->Local_Fire(Hit.ImpactPoint, Hit.ImpactNormal, ImpactSurfaceType, true);
	
	OnRoundFired.Broadcast(CurrentWeapon->Ammo, CurrentWeapon->MagCapacity, CurrentReserveAmmo);
	
	// 발사 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		FireTimer, 
		this, 
		&ThisClass::FireTimerFinished, 
		CurrentWeapon->FireTime
		);
	// 서버 무기 발사 
	Server_FireWeapon(Hit);
}

int32 UCombatComponent::AdvanceWeaponIndex()
{
	// Weapon이 두 개 이상일 때만 다음 Index로 이동한다.
	if (Inventory.Num() >= 2)
	{
		Local_WeaponIndex = (Local_WeaponIndex + 1) % Inventory.Num();
	}
	// Weapon이 하나뿐이면 기존 Index를 그대로 반환한다.
	return Local_WeaponIndex;
}



void UCombatComponent::FireTimerFinished()
{
	if (!IsValid(CurrentWeapon)) return;
	
	if (bTriggerPressed && CurrentWeapon->FireType == EFireType::Auto && CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::Server_FireWeapon_Implementation(const FHitResult& Hit)
{
	
	if (!IsValid(CurrentWeapon)) return;
	
	// Listen Server Host의 중복 Ammo 감소 방지 - 예외처리
	// 내가 방장이 아니거나, 내가 직접 조종하는 캐릭터가 아니라면 서버에서 총을 쏴라
	if (GetNetMode() != NM_ListenServer || !Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		CurrentWeapon->Auth_Fire();
	}
	
	Multicast_FireWeapon(Hit, CurrentWeapon->Ammo);
}

void UCombatComponent::Multicast_FireWeapon_Implementation(const FHitResult& Hit, int32 AuthAmmo)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (OwningPawn->IsLocallyControlled())
	{
		CurrentWeapon->Rep_Fire(AuthAmmo);
	}
	else
	{
		ensure(IsValid(WeaponData));
		
		EPhysicalSurface ImpactSurfaceType = 
		Hit.PhysMaterial.IsValid(false) 
	? Hit.PhysMaterial->SurfaceType.GetValue() 
	: SurfaceType1;
	
		CurrentWeapon->Local_Fire(Hit.ImpactPoint, Hit.ImpactNormal, ImpactSurfaceType, false);
		
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
	bTriggerPressed = false;
}

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

void UCombatComponent::OnRep_CurrentReserveAmmo()
{
	// 서버로부터 최신 예비 탄약수(CurrentReserveAmmo)를 전송받은 클라이언트가, 
	// 자신의 화면(UI)을 최신 탄약 정보로 즉시 갱신하기 위해 이벤트를 터뜨리는(브로드캐스트) 코드
	if (IsValid(CurrentWeapon))
	{
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	}
}

// Aim 상태 변경 시 실행되는 로컬 처리를 하나의 진입점으로 모으는 것 
void UCombatComponent::Local_Aim(bool bPressed)
{
	bAiming = bPressed;
	OnAimingStatusChanged.Broadcast(bAiming);
}



void UCombatComponent::Equip(AWeapon* Weapon)
{
	// 서버에서 현재 Weapon을 권위 있게 설정한다.
	CurrentWeapon = Weapon;			
	
	// Server가 알고 있는 CombatComponent Owner Pawn을 명시적으로 전달한다.
	CurrentWeapon->AttachToOwningPawn(Cast<APawn>(GetOwner()));	
	
	// 현재 장착한 무기 종류에 맞는 예비 탄약수를 조회하여 변수에 저장하고, ( WeaponType은 ReserveAmmo에 반드시 존재 ) 
	// 탄약 변경 사항을 UI 등 다른 시스템에 알리기 위해 이벤트를 발생(브로드캐스트)
	CurrentReserveAmmo = ReserveAmmo.FindChecked(CurrentWeapon->WeaponType);
	OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, Weapon->Ammo, CurrentWeapon->WeaponIcon);
}

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	// 유효한 Weapon과 Owner가 있어야 교체를 진행할 수 있다.
	if (!IsValid(Weapon) || !IsValid(GetOwner()))
	{
		return;
	}

	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		// Server/Listen Host는 Replicated CurrentWeapon을 직접 변경한다.
		SetCurrentWeapon(Weapon, CurrentWeapon);
	}
	else
	{
		// Owning Client는 Server에 Weapon 변경을 요청한다.
		Server_EquipWeapon(Weapon);
	}
}

void UCombatComponent::Server_EquipWeapon_Implementation(AWeapon* Weapon)
{
	// Client가 요청한 Weapon Equip을 Server 권위 경로에서 다시 실행한다.
	EquipWeapon(Weapon);
}

void UCombatComponent::SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon)
{
	// Server 직접 호출과 Client RepNotify 모두에서 사용할 실제 이전 Weapon을 결정한다.
	AWeapon* LocalLastWeapon = nullptr;

	if (IsValid(LastWeapon))
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// 이전 Weapon이 있다면 Pawn에서 제거하고 Inventory 보관 상태로 전환한다.
	if (IsValid(LocalLastWeapon))
	{
		LocalLastWeapon->DetachFromOwningPawn();
		LocalLastWeapon->WeaponStatus = EWeaponStatus::Unequipped;
	}
	
	// 실제 CurrentWeapon Reference를 새 Weapon으로 변경한다.
	CurrentWeapon = NewWeapon;
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	
	// ReserveAmmo Map은 Server 권위 데이터이므로 Server에서만 현재 Reserve Snapshot을 갱신한다.
	if (IsValid(OwningPawn) && OwningPawn->HasAuthority() && IsValid(CurrentWeapon))
	{
		CurrentReserveAmmo = ReserveAmmo.FindChecked(CurrentWeapon->WeaponType);
	}
	
	// 새 CurrentWeapon을 Pawn의 1P / 3P Mesh에 Attach한다.
	CurrentWeapon->AttachToOwningPawn(OwningPawn);
}

// 인벤토리 생성
void UCombatComponent::SpawnInventory()
{
	// 권한 점검 - 서버가 아니면 즉시 종료 
	if(GetOwner()->GetLocalRole() < ROLE_Authority) return;

	
	// 기본 무기 목록(DefaultWeaponClasses)을 순회하며 무기를 하나씩 생성(Spawn)한 뒤, 
	// 인벤토리에 추가하고 각 무기 타입별 시작 예비 탄약수(StartingCarriedAmmo)를 초기화
	for(TSubclassOf<AWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		AWeapon* Weapon = SpawnWeapon(WeaponClass);
		Inventory.AddUnique(Weapon);	// 동일 포인터 중복만 거름 
		ReserveAmmo.Add(Weapon->WeaponType, Weapon->StartingCarriedAmmo);
	}

	if(Inventory.Num() > 0)		// 빈 배열 인덱싱 방지
	{
		Equip(Inventory[0]);
		InitializeWeaponWidgets();
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

void UCombatComponent::InitializeWeaponWidgets() const
{
	if (IsValid(CurrentWeapon))
	{
		OnReticleChanged.Broadcast(
			CurrentWeapon->GetReticleDynamicMaterialInstance(), 
			CurrentWeapon->ReticleParams, 
			bHitPlayer
			);
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynamicMaterialInstance(), 
			CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
	}
	
}




void UCombatComponent::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	/*//	게임 중 무기를 바꾸는 상황은 OnRep_CurrentWeapon이 맡는다
	if (!IsValid(CurrentWeapon)) return;

	// "현재 Weapon이 바뀌었다"는 정확한 Event에서 해당 Weapon만 Attach한다.
	CurrentWeapon->AttachToOwningPawn(Cast<APawn>(GetOwner()));*/
	
	// Client에서도 Server와 동일한 Old/New Weapon 정리 절차를 재사용한다.
	SetCurrentWeapon(CurrentWeapon, LastWeapon);

	
	// 인터페이스 호출
	IPlayerInterface::Execute_WeaponReplicated(GetOwner());
	InitializeWeaponWidgets();
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





