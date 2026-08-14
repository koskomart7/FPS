// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API UWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	// 해당 Tag에 맞는 무기 GripPoint 데이터를 맵 형태로 관리 
	UPROPERTY(EditAnywhere, Category = "FPS|WeaponData|Weapons")
	TMap<FGameplayTag, FName> GripPoints;
	
};
