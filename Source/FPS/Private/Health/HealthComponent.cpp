// Copyright DevS


#include "Health/HealthComponent.h"

#include "Net/UnrealNetwork.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	DeathState = EDeathState::NotDead;
	SetIsReplicatedByDefault(true);
	
	Health = 100.f;
	MaxHealth = 100.f;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Death는 다른 Player들도 알아야 하므로 일반 Replication.
	DOREPLIFETIME(UHealthComponent, DeathState);
	// 정확한 Health 수치는 Owner UI에만 필요하므로 OwnerOnly.
	DOREPLIFETIME_CONDITION(UHealthComponent, Health, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UHealthComponent, MaxHealth, COND_OwnerOnly);
}

float UHealthComponent::GetHealthNormalized() const
{
	// MaxHealth가 0 이하라면 Division by Zero를 피하고 0을 반환한다.
	return (MaxHealth > 0.f) ? ( Health/MaxHealth) : 0.f;
}

bool UHealthComponent::ChangeHealthByAmount(float Amount, AActor* Instigator)
{
	float OldValue = Health;
	// 양수면 Healing, 음수면 Damage가 되며 Health 범위는 0~MaxHealth로 제한한다.
	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	// Server/local listener에게 Old/New 값과 변화 원인을 전달한다.
	OnHealthChanged.Broadcast(this, OldValue, Health, Instigator);
	
	// Check if lethal -> start Death
	if (Health <= 0.f)
	{
		// 사망 시작과 동시에 이번 Health 변경을 Lethal로 보고한다.
		StartDeath();
		return true;
	}
	
	return false;
}

void UHealthComponent::StartDeath()
{
	// 이미 DeathStarted/DeathFinished라면 다시 사망을 시작하지 않는다.
	if (DeathState != EDeathState::NotDead)
	{
		return;
	}
	
	DeathState = EDeathState::DeathStarted;
	OnDeathStarted.Broadcast();
	GetOwner()->ForceNetUpdate();	// 즉시 다음 네트워크 틱(Tick)에 리플리케이션을 수행하도록 엔진에 요청하는 함수
}



void UHealthComponent::ChangeMaxHealthByAmount(float Amount, AActor* Instigator)
{
	float OldValue = MaxHealth;
	MaxHealth += Amount;
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, Instigator);
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

		
}

void UHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	// Client가 DeathStarted 상태를 수신하면 Server와 같은 Delegate Contract를 사용한다.
	if (DeathState == EDeathState::DeathStarted)
	{
		OnDeathStarted.Broadcast();
	}
}

void UHealthComponent::OnRep_Health(float OldValue)
{
	OnHealthChanged.Broadcast(this, OldValue, Health, nullptr);
}

void UHealthComponent::OnRep_MaxHealth(float OldValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, nullptr);
}





