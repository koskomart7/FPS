#pragma once

#include "ShooterTypes.generated.h"

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	Left UMETA(DisplayName = "Turning Left"),
	Right UMETA(DisplayName = "Turning Right"),
	NotTurning UMETA(DisplayName = "Not Turning")
};

USTRUCT(BlueprintType)
struct FReticleParams
{
	GENERATED_BODY()
	
	// Shape Cut Factor
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_RoundFired = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_Aiming = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_NotAiming = 0.f;
	
	// Scale Factor
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_RoundFired = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Aiming = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_NotAiming = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Targeting = 0.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_NotTargeting = 0.f;
	
	// Interp Speeds
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RoundFiredInterpSpeed = 20.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AimingInterpSpeed = 15.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TargetingPlayerInterpSpeed = 10.f;
};

// << : 비드 왼쪽 시프트 연산자 -> 왼쪽 변수의 이진수 비트들을 오른쪽 변수의 숫자만큼 왼쪽으로 밀어내는 역할
// 비트 연산자 | (OR)를 사용해 Headshot | Sequential과 같이 두 가지 속성을 하나의 변수에 모두 담을 수 있기 때문입니다. 
// (이진수로는 00000000 00000011이 됩니다.)
UENUM(meta = (BitFlags))	// 이 Enum을 Bit Flag 용도로 사용하는 Type
enum class ESpecialElimType : uint16 
{
	None			= 0,			// 모든 비트가 0인 상태입니다. (00000000 00000000)
	Headshot		= 1 << 0,		// 00000000 00000001 숫자 1을 왼쪽으로 0칸 이동합니다. (1 × 2⁰ = 1)
	Sequential		= 1 << 1,		// 00000000 00000010 숫자 1을 왼쪽으로 1칸 이동합니다. (1 × 2¹ = 2)
	Streak			= 1 << 2,		// 00000000 00000100
	Revenge			= 1 << 3,		// 00000000 00001000
	Dethrone		= 1 << 4,		// 00000000 00010000
	Showstopper		= 1 << 5,		// 00000000 00100000
	FirstBlood		= 1 << 6,		// 00000000 01000000
	GainedTheLead	= 1 << 7,		// 00000000 10000000
	TiedTheLeader	= 1 << 8,		// 00000001 00000000
	LostTheLead		= 1 << 9		// 00000010 00000000 숫자 1을 왼쪽으로 9칸 이동합니다. (1 × 2⁹ = 512)
};

// Macro - Enum Class를 Flag로 사용할 때 필요한 Bitwise Operator들을 자동 정의
ENUM_CLASS_FLAGS(ESpecialElimType)
