// Copyright DevS


#include "Character/HeroCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Combat/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
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
	
	TurningStatus = ETurningInPlace::NotTurning;
}


void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	FirstPersonCamera->SetFieldOfView(DefaultFOV);
	
	StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
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

bool AHeroCharacter::HasCurrentWeapon() const
{
	return IsValid(Combat) && Combat->CurrentWeapon != nullptr;
}

void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateTurnInPlaceParameters(DeltaTime);
	CalculateFABRIKSocketTransform();
}

void AHeroCharacter::CalculateTurnInPlaceParameters(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.f && !bIsInAir)	// Standing still, not jumping
	{
		FRotator CurrentAimRotation(0.f, GetBaseAimRotation().Yaw, 0.f);
		
		// StartingAimRotation은 BeginPlay 에서 초기화된다 
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		
		AO_Yaw = DeltaAimRotation.Yaw;
		
		if (TurningStatus == ETurningInPlace::NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		
		TurnInPlace(DeltaTime);
	}
	
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		
		AO_Yaw = 0.f;
		
		FRotator AimRotation = GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());
		
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		TurningStatus = ETurningInPlace::NotTurning;
	}
	
	// 위의 모든 로직 계산후 애니메이션에서 방향이 반대로 나타나는것을 수정하기 위함	
	AO_Yaw *= -1.f;
	
	
	// Get velocity, see if it's zero
	// See if we are falling
	
	// if standing still and not jumping
		// get current aim rotation
		// get delta aim rotation - the difference in rotation of my current aim rotation from the initial aim rotation
		// ( initial aim rotation is calculated in BeginPlay )
		// Store the Yaw of the delta aim rotation ( AO_Yaw )
		// if TurningStatus == NotTurning
			// Set InterpAO_Yaw to AO_Yaw
		// TurnInPlace() - interpolates the InterpAO_Yaw value to zero.
	
	// if running or jumping
		// reset initial aim rotation to the current actual aim rotation
		// AO_Yaw = 0
		// We also need a Movement Offset Yaw to feed to our strafing blendspaces
		// Get Base Aim Rotation
		// Get our Movement Rotation - this is the rotation of our Velocity
		// Movement Offset Yaw = the delta between our movement rotation and our aim rotation
		// TurningStatus = NotTurning
		
// ============================== 한글 버전 ====================================
	// 1. 상태 확인
            // 속도(Velocity)를 가져와 0인지(멈춰있는지) 확인합니다.
            // 현재 공중에 떠 있는 상태(Falling)인지 확인합니다.
	
	// 2. 가만히 서 있고 점프 중이 아닐 때 (정지 상태)
            // if (가만히 서 있음 && 점프 중이 아님)
            //{
                // 현재 조준 회전값(Aim Rotation)을 가져옵니다.
                // 델타 조준 회전값(Delta Aim Rotation)을 계산합니다.
                // -> (현재 조준 회전값 - 기준이 되는 최초 조준 회전값)의 차이입니다.
                // -> 참고: 최초 조준 회전값은 게임 시작 시(BeginPlay) 계산됩니다.
        
                // 델타 조준 회전값의 좌우 회전값(Yaw)을 AO_Yaw에 저장합니다.
                // (AO = Aim Offset, 조준 오프셋)
        
                // 만약 현재 회전 상태가 '회전하지 않음(NotTurning)'이라면
                // {
                //     InterpAO_Yaw 값을 현재 AO_Yaw 값으로 설정합니다.
                // }
        
                // TurnInPlace() 함수를 실행합니다.
                // -> 이 함수는 InterpAO_Yaw 값을 0으로 서서히 보간(부드럽게 감소)시킵니다.
            //}
        
	// 3. 달리는 중이거나 점프 중일 때 (이동 상태)
            // if (달리는 중 || 점프 중)
            //{
                // 최초 조준 회전값을 현재의 실제 조준 회전값으로 초기화합니다.
                // AO_Yaw 값을 0으로 만듭니다.
            //}
        
	// 4. 꽃게걸음(Strafing) 블렌드 스페이스에 전달할 이동 오프셋 Yaw 계산
            // 기본 조준 회전값(Base Aim Rotation)을 가져옵니다.
            // 이동 회전값(Movement Rotation)을 가져옵니다. (이것은 현재 속도(Velocity)의 방향입니다.)
            // 이동 오프셋 Yaw(Movement Offset Yaw) = 이동 회전값과 조준 회전값의 차이(Delta)를 계산합니다.
            
            // 현재 상태를 '회전하지 않음(NotTurning)'으로 설정합니다.
}

void AHeroCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningStatus = ETurningInPlace::Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningStatus = ETurningInPlace::Left;
	}
	
	// 왼쪽이나 오른쪽으로 회전중이라면
	if (TurningStatus != ETurningInPlace::NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(
			InterpAO_Yaw,
			0.f,
			DeltaTime,
			4.f
			);
		
		AO_Yaw = InterpAO_Yaw;
		
		if (FMath::Abs(AO_Yaw) < 5.f)
		{
			TurningStatus = ETurningInPlace::NotTurning;
			
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

// Turn In Place
	// if AO_Yaw > 90
		// TurningStatus = Right
	// else if AO_Yaw < -90
		// TurningStatus = Left
	// if TurningStatus != NotTurning ( in other words, we are turning left or right )
		// Interpolate InterpAO_Yaw down to zero.
		// AO_Yaw = InterpAO_Yaw
		// if Abs(AO_Yaw) < 5.f
			// TurningStatus = NotTurning
			// reset initial aim rotation to our actual aim rotation
			
// --- 제자리 회전 처리 함수 (Turn In Place) 내부 로직 ---
	// 만약 AO_Yaw 값이 90도보다 크다면
	// {
	//     회전 상태(TurningStatus) = 오른쪽(Right)으로 설정합니다.
	// }
	// 그렇지 않고 만약 AO_Yaw 값이 -90도보다 작다면
	// {
	//     회전 상태(TurningStatus) = 왼쪽(Left)으로 설정합니다.
	// }

	// 만약 회전 상태가 '회전하지 않음'이 아니라면 (즉, 왼쪽이나 오른쪽으로 회전 중이라면)
	// {
	//     InterpAO_Yaw 값을 0을 향해 서서히 줄여나갑니다(보간).
	//     AO_Yaw 값을 현재 줄어들고 있는 InterpAO_Yaw 값으로 업데이트합니다.

	//     만약 AO_Yaw의 절대값(Abs)이 5도 미만으로 떨어지면 (회전이 거의 끝났다면)
	//     {
	//         회전 상태(TurningStatus) = 회전하지 않음(NotTurning)으로 설정합니다.
	//         최초 조준 회전값을 현재의 실제 조준 회전값으로 다시 초기화합니다.
	//     }
	// }

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



