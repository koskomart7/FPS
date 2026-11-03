// Copyright DevS

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

class AHeroPlayerState;
class UTextBlock;

UCLASS()
class FPS_API UScoreWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Widget Instance 당 한 번만 수행할 Setup에 적합
	virtual void NativeOnInitialized() override;
	
	// Score text 갱신
	UPROPERTY(meta = (BindWideget))
	TObjectPtr<UTextBlock> Text_Score;
	
private:
	// 초기 PlayerState 획득
	AHeroPlayerState* GetPlayerState() const;
	
	UFUNCTION()
	void OnScoreChanged(int32 Score);
	
	UFUNCTION()
	void OnPlayerStateReplicated();
	
};
