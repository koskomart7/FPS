// Copyright DevS


#include "Character/HeroCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Combat/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
}


void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
}

void AHeroCharacter::Input_Aim_Released()
{
	Combat->Initiate_Aim_Released();
}

