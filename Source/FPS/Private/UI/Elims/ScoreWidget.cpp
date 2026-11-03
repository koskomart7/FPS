// Copyright DevS


#include "UI/Elims/ScoreWidget.h"

#include "Components/TextBlock.h"
#include "Player/HeroPlayerController.h"
#include "Player/HeroPlayerState.h"

void UScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	AHeroPlayerState* PS = GetPlayerState();
	if (IsValid(PS))
	{
		PS->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);
	}
	else
	{
		// PlayerState가 언제 유효한지 판단하기 위한 로직
		// PlayerState의 리플리케이션이 완료되었을 때 PC에서 델리게이트 알림
		AHeroPlayerController* PC = Cast<AHeroPlayerController>(GetOwningPlayer());
		if (IsValid(PC))
		{
			// “PlayerState 도착 대기”용 일회성 Binding이라는 의도 - AddUniqueDynamic()은 중복 바인딩을 허용하지 않는다.
			PC->OnPlayerStateReplicated.AddUniqueDynamic(this, &ThisClass::OnPlayerStateReplicated);
		}
	}
}

void UScoreWidget::OnPlayerStateReplicated()
{
	AHeroPlayerState* PS = GetPlayerState();
	if (IsValid(PS))
	{
		// 이제 PlayerState가 유효하므로 실제 ScoreEvent에 연결한다.
		PS->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);
		
		// 최초 Text 값을 한 번 직접 갱신한다.
		OnScoreChanged(PS->GetScoredElims());
	}
	
	AHeroPlayerController* PC = Cast<AHeroPlayerController>(GetOwningPlayer());
	if (IsValid(PC))
	{
		// PlayerState를 얻었으므로 임시 Replication 대기 Delegate는 제거한다.
		PC->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
}

AHeroPlayerState* UScoreWidget::GetPlayerState() const
{
	APlayerController* PC = GetOwningPlayer();
	if (IsValid(PC))
	{
		return PC->GetPlayerState<AHeroPlayerState>();
	}
	return nullptr;
}

void UScoreWidget::OnScoreChanged(int32 Score)
{
	if (IsValid(Text_Score))
	{
		// 숫자를 사용자 UI용 FText로 변환해 Score Text를 갱신한다.
		Text_Score->SetText(FText::AsNumber(Score));
	}
}
