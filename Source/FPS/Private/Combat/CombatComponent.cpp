// Copyright DevS


#include "Combat/CombatComponent.h"
#include "GameFramework/Pawn.h"
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
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f, 
		FColor::Cyan, 
		TEXT("Initiate_Aim_Pressed"),
		false);	
}

void UCombatComponent::Initiate_Aim_Released()
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f, 
		FColor::Cyan, 
		TEXT("Initiate_Aim_Released"),
		false);	
}

void UCombatComponent::SpawnInventory()
{
	AWeapon* NewWeapon = SpawnWeapon(DefaultWeaponClass);
}

void UCombatComponent::DestroyInventory()
{
	// TODO: 재고가 확보되는 대로 이를 폐기하십시오.
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

