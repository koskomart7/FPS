// Copyright DevS


#include "Weapon/Weapon.h"

#include "KismetTraceUtils.h"
#include "Components/SkeletalMeshComponent.h"
#include "FPS/FPS.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/PlayerInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;  // 무기의 소유자와 관련이 없다면 볼수 없다 ( 실소유자를 만나면 보이게 된다 ) 
	
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh1P");
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetHiddenInGame(true);		// 장착 되지 않는 재고 무기를 숨기기 위해
	SetRootComponent(Mesh1P);
	
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh3P");
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetupAttachment(Mesh1P);
	Mesh3P->SetHiddenInGame(true);		// 장착 되지 않는 재고 무기를 숨기기 위해
	
	AimFOV = 65.0f;
	TraceRadius = 5.f;
	FireTime = 0.1f;
	
	MagCapacity = 10;
	Ammo = 5;
	StartingCarriedAmmo = 10;
	Sequence = 0;
	
}

void AWeapon::OnRep_Instigator()
{
	// 게임 시작시 여러 오브젝트의 복제 순서는 비결정적
	// 무기 스스로도 유효한 Instigator가 생기는 시점에 부착을 시도하도록 유지 - OnRep_Instigator
	// 그 이후, 게임 중 무기를 바꾸는 상황은 OnRep_CurrentWeapon이 맡는다
	Super::OnRep_Instigator();
	AttachToOwningPawn();
}

USkeletalMeshComponent* AWeapon::GetMesh1P() const
{
	return Mesh1P;
}

USkeletalMeshComponent* AWeapon::GetMesh3P() const
{
	return Mesh3P;
}

UMaterialInstanceDynamic* AWeapon::GetReticleDynamicMaterialInstance()
{
	if (!IsValid(DynMatInst_Reticle))
	{
		DynMatInst_Reticle = UMaterialInstanceDynamic::Create(ReticleMaterial, this);
	}
	return DynMatInst_Reticle;
}

UMaterialInstanceDynamic* AWeapon::GetAmmoCounterDynamicMaterialInstance()
{
	if (!IsValid(DynMatInst_AmmoCounter))
	{
		DynMatInst_AmmoCounter = UMaterialInstanceDynamic::Create(AmmoCounterMaterial, this);
	}
	return DynMatInst_AmmoCounter;
}

void AWeapon::AttachToOwningPawn() const
{
	// 소유자 검사
	APawn* OwingPawn = GetInstigator();
	if (!IsValid(OwingPawn) || !OwingPawn->Implements<UPlayerInterface>()) return;

	// 메쉬 가시성 설정
	SetMeshVisibilities(OwingPawn);	// const 함수 내부에서 호출시에 해당 함수도 const 여야함

	// BNE( Blueprint Native Event ) 인터페이스 함수 호출 _ Execute_ 정적 래퍼 
	const FName AttachPoint = IPlayerInterface::Execute_GetWeaponAttachPoint(OwingPawn, WeaponType);
	USkeletalMeshComponent* PawnMesh1P = IPlayerInterface::Execute_GetMesh1P(OwingPawn);
	USkeletalMeshComponent* PawnMesh3P = IPlayerInterface::Execute_GetMesh3P(OwingPawn);
	
	// 메쉬 소켓에 부착
	Mesh1P->AttachToComponent(PawnMesh1P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
	Mesh3P->AttachToComponent(PawnMesh3P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);

}

void AWeapon::WeaponTrace(FHitResult& OutHit, float TraceLenght)
{
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(GetOwner());	// 자기 자신을 Trace 대상에서 제외
	
	// 어떤 Object Type을 맞출 것인가
	FCollisionResponseParams ResponseParams;
	// 모든 채널 Ignore
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	// 필요한 채널만 block
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_WorldStatic, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
	
	ensure(GetInstigator());
	if (APlayerController* PC = Cast<APlayerController>(GetInstigator()->GetController()); IsValid(PC))
	{
		FVector EyesWorldLocation;
		FRotator EyesWorldRotation;
		PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
		
		const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
		const FVector Start = EyesWorldLocation;
		const FVector End = Start + EyesWorldDirection * TraceLenght;
		
		const bool bHit = GetWorld()->SweepSingleByChannel(
			OutHit,
			Start,
			End,
			FQuat::Identity,
			FPSTraceChannels::ECC_Weapon,
			FCollisionShape::MakeSphere(TraceRadius),
			QueryParams,
			ResponseParams);
		
		if (!bHit)
		{
			OutHit.ImpactPoint = End;
		}
	}
	
}

void AWeapon::Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal,
	TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson)
{
	FireEffects(ImpactPoint, ImpactNormal, ImpactSurfaceType, bIsFirstPerson);
	
	// 해당 로컬 플레이어에서만 실행 
	if (GetInstigator()->IsLocallyControlled())
	{
		Ammo = FMath::Clamp(Ammo-1, 0, MagCapacity);
		++Sequence;
	}
}

void AWeapon::Auth_Fire()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
}

void AWeapon::Rep_Fire(int32 AuthAmmo)
{
	if (GetInstigator()->IsLocallyControlled())
	{
		Ammo = AuthAmmo;
		--Sequence;
		Ammo -= Sequence;
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::SetMeshVisibilities(APawn* OwningPawn) const
{
	// 현재 이 머신에서 해당 폰을 조종 중인가 - 무기는 코드가 직접 판정
	if(OwningPawn->IsLocallyControlled())
	{
		Mesh1P -> SetHiddenInGame(false);	// 내화면 : 1인칭 무기만 보임
		Mesh3P -> SetHiddenInGame(true);
	}
	else
	{
		Mesh1P -> SetHiddenInGame(true);	// 상대 / 서버 관점 : 3인칭 무기만 보임
		Mesh3P -> SetHiddenInGame(false);
	}
}
