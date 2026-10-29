// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShooterTypes/ShooterTypes.h"
#include "SpecialElimData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FSpecialElimInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	ESpecialElimType ElimType = ESpecialElimType::None;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString ElimMessage = FString();
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UTexture2D> ElimIcon = nullptr;
	
	// 매 Kill 마다 달라지는 런타임 값이기에 EditDefaultOnly 사용 안함 )
	UPROPERTY(BlueprintReadOnly)
	int32 SequentialElimCount = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 StreakCount = 0;
};

UCLASS()
class FPS_API USpecialElimData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS|SpecialElims")
	TMap<ESpecialElimType, FSpecialElimInfo> SpecialElimInfo;
	
};
