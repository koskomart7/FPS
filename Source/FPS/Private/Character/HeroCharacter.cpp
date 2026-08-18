// Copyright DevS


#include "Character/HeroCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Combat/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon/Weapon.h"

// Sets default values
AHeroCharacter::AHeroCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	
	GetCharacterMovement()->MovementState.bCanCrouch = true;
	
	// SpringArm :  컨트롤러 회전을 따라가는 회전 허브
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("springArm");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 0.f;		// 1인칭 이므로 거리 0
	SpringArm->bEnableCameraLag = true;		// 부드러운 보간 
	SpringArm->CameraLagSpeed = 15.f;
	SpringArm->bUsePawnControlRotation = true;	// 컨트롤러 회전 따라감
	
	// 카메라 : 스프링암에 부착, 회전은 위임
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>("FirstPersonCamera");
	FirstPersonCamera->SetupAttachment(SpringArm);
	FirstPersonCamera->bUsePawnControlRotation = false;		// 이중 회전 방지
	
	// Mesh1P : 1인칭 팔 (카메라에 부착) - 시점 따라 움직임
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh1");
	Mesh1P->SetupAttachment(FirstPersonCamera);
	Mesh1P->bOnlyOwnerSee = true;					// ★ 나만 보임
	Mesh1P->bOwnerNoSee = false;
	Mesh1P->bCastDynamicShadow = false;				// 팔 그림자 제거 (최적화)
	Mesh1P->bReceivesDecals = false;				// 총알 데칼이 팔에 안 붙게 , 피가 튀거나 할때는 true
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered; //렌더될때만 애니계산
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;		 // 물리 전에 갱신 -> 떨림 방지
	
	// Mesh(상속_전신) : Mesh1P와 정반대 가시성
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;		// ★ 나는 안 보임
	GetMesh()->bReceivesDecals = false;
	
	Combat = CreateDefaultSubobject<UCombatComponent>("Combat");
	Combat->SetIsReplicated(true);
	
	DefaultFOV = 90.0f;
}


void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	FirstPersonCamera->SetFieldOfView(DefaultFOV);
}

void AHeroCharacter::BeginDestroy()
{
	Super::BeginDestroy();
	
	if (IsValid(Combat))
	{
		Combat->DestroyInventory();
	}
}

FRotator AHeroCharacter::GetFixedAnimRotation() const
{
	// 엔진이 이미 계산한 AimRotation 사용
	FRotator AimRotation = GetBaseAimRotation();
	// Pitch 값이 90 보다 크고, 자기 자신이 조종하는 Pawn이 아닐때
	if (AimRotation.Pitch > 90.0f && !IsLocallyControlled())
	{
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		// 입력 범위 안에서 값의 상대적 위치를 계산, 출력 범위에서 같은 비율의 값을 반환, 값 범위 제한
		AimRotation.Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimRotation.Pitch);
	}
	
	return AimRotation;
}

void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateFABRIKSocketTransform();
}

void AHeroCharacter::CalculateFABRIKSocketTransform()
{
	if (IsValid(Combat) 
		&& IsValid(Combat->CurrentWeapon) 
		&& IsValid(Combat->CurrentWeapon->GetMesh3P()))
	{
		// FABRIK_Socket의 World-space Transform 계산
		// RTS_World 사용 이유 : Weapon의 Socket과 캐릭터의 hand_r은 서로 다른 SkeletalMesh Component에 있기 때문
		// 두 컴포넌트가 공유하는 공통기준 = World Space
		FABRIK_SocketTransform = 
			Combat->CurrentWeapon->GetMesh3P()
		->GetSocketTransform("FABRIK_Socket", RTS_World);
		
		FVector OutLocation;
		FRotator OutRotation;
		
		// FQuat 를 FRotator로 변환
		// World Space의 Location/Rotation을 특정 Bone 기준 Relative Space로 변환.
		// 여기서는 Weapon의 Socket을 캐릭터의 Bone 공간으로 변환
		GetMesh()->TransformToBoneSpace(
			"hand_r",
			FABRIK_SocketTransform.GetLocation(),
			FABRIK_SocketTransform.GetRotation().Rotator(),	
			OutLocation,
			OutRotation);
		
		// FTransform 내부 Rotation은 Quaternion 기반이므로 FRotator를 다시 FQuat로 바꾼다.
		FABRIK_SocketTransform.SetLocation(OutLocation);
		FABRIK_SocketTransform.SetRotation(OutRotation.Quaternion());
	}
}

void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* HeroInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	HeroInputComponent->BindAction(CycleWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_CycleWeapon);
	HeroInputComponent->BindAction(ReloadWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_ReloadWeapon);
	HeroInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_FireWeapon_Pressed);
	HeroInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &ThisClass::Input_FireWeapon_Released);
	HeroInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_Aim_Pressed);
	HeroInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Completed, this, &ThisClass::Input_Aim_Released);
	
}

void AHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// BeginPlay 시점엔 빙의가 보장되지 않는다. -> 따라서 PossessedBy로 빙의 완료 확인 
	if (IsValid(Combat))
	{
		Combat->SpawnInventory();
	}
}

// 무기 부착 인터페이스
FName AHeroCharacter::GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponType) const
{
	checkf(Combat->WeaponData, TEXT("No Weapon Data Asset - Please fill out BP_HeroCharacter"));
	return Combat->WeaponData->GripPoints.FindChecked(WeaponType);
}

USkeletalMeshComponent* AHeroCharacter::GetMesh3P_Implementation() const
{
	return GetMesh();
}

USkeletalMeshComponent* AHeroCharacter::GetMesh1P_Implementation() const
{
	return Mesh1P;
}



void AHeroCharacter::Input_CycleWeapon()
{
	Combat->Initiate_CycleWeapon();
}

void AHeroCharacter::Input_ReloadWeapon()
{
	Combat->Initiate_ReloadWeapon();
}

void AHeroCharacter::Input_FireWeapon_Pressed()
{
	Combat->Initiate_FireWeapon_Pressed();
}

void AHeroCharacter::Input_FireWeapon_Released()
{
	Combat->Initiate_FireWeapon_Released();
}

void AHeroCharacter::Input_Aim_Pressed()
{
	Combat->Initiate_Aim_Pressed();
	OnAim(true);
}

void AHeroCharacter::Input_Aim_Released()
{
	Combat->Initiate_Aim_Released();
	OnAim(false);
	
}



