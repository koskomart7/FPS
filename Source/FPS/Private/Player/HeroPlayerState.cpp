// Copyright DevS


#include "Player/HeroPlayerState.h"

#include "Data/SpecialElimData.h"
#include "UI/Elims/SpecialElim.h"

AHeroPlayerState::AHeroPlayerState()
{
	// UE 5.8에서 제공하는 Setter를 통해 Network Update Frequency를 설정한다.
	SetNetUpdateFrequency(100.f);
	
	ScoredElims = 0;
	Defeats = 0;
	Hits = 0;
	Misses = 0;
	bOnStreak = false;
	HeadShotElims = 0;
	HighestStreak = 0;
	RevengeElims = 0;
	DethroneElims = 0;
	ShowStopperElims = 0;
	bFirstBlood = false;
	bWinner = false;
	bIsProcessingQueue = false;
	ElimDisplayTime = 0.5f;
}

void AHeroPlayerState::AddScoredElim()
{
	++ScoredElims;
}

void AHeroPlayerState::AddDefeat()
{
	++Defeats;
}

void AHeroPlayerState::AddHit()
{
	++Hits;
}

void AHeroPlayerState::AddMiss()
{
	++Misses;
}

void AHeroPlayerState::AddHeadShotElim()
{
	++HeadShotElims;
}

void AHeroPlayerState::AddSequentialElim(int32 SequenceCount)
{
	/*
	Value: Match에서 해당 Sequence를 달성한 횟수
	예:
	{
		2 : 3,   // Double Kill 3회
		3 : 1,   // Triple Kill 1회
		4 : 2    // Quad Kill 2회
	}
	*/
	
	if (SequentialElims.Contains(SequenceCount))
	{
		// 이미 기록이 있으면 해당 Sequence 달성 횟수를 증가시킨다.
		SequentialElims[SequenceCount]++;
	}
	else
	{
		// 처음 달성한 Sequence라면 1회로 등록한다.
		SequentialElims.Add(SequenceCount, 1);
	}
	
	/* Reduce the count for all lower sequence counts
	 * this is because a triple elim means a double was
	 * scored first. But we want to count this as just a triple,
	 * i.e. elim 1, elim 2, elim 3 = just a triple, not a double and a triple
	 
	* 모든 낮은 순서 점수의 합계를 줄이세요.
	 * 이는 트리플 엘리미네이션이 더블이 먼저 기록되었음을 의미하기 때문입니다.
	 * 하지만 우리는 이를 단순히 트리플로만 계산하고자 합니다.
	 * 즉, 엘리미네이션 1, 엘리미네이션 2, 엘리미네이션 3은 단순히 트리플일 뿐, 더블과 트리플이 따로 있는 것이 아닙니다.
	 * Triple이 되면 그 Chain에서 직전에 기록한 Double을 취소	
	 */
	for (auto& Elem : SequentialElims)
	{
		// 현재 Sequence보다 낮은 기록 중 / 양수인 항목을 하나씩 차감한다.
		if (Elem.Key < SequenceCount && Elem.Value > 0)
		{
			Elem.Value--;
		}
	}
	
}

void AHeroPlayerState::UpdateHighestStreak(int32 StreakCount)
{
	if (StreakCount > HighestStreak)
	{
		HighestStreak = StreakCount;
	}
}

void AHeroPlayerState::AddRevengeElim()
{
	++RevengeElims;
}

void AHeroPlayerState::AddDethroneElim()
{
	++DethroneElims;
}

void AHeroPlayerState::AddShowStopperElim()
{
	++ShowStopperElims;
}

void AHeroPlayerState::GotFirstBlood()
{
	bFirstBlood = true;
}

void AHeroPlayerState::IsNowWinner()
{
	bWinner = true;
}

void AHeroPlayerState::SetOnStreak(bool bIsOnStreak)
{
	// Streak 진입/종료 상태를 Server PlayerState에 기록한다.
	bOnStreak = bIsOnStreak;
}

void AHeroPlayerState::SetLastAttacker(APlayerState* Attacker)
{
	// Revenge 판정을 위해 마지막 killer를 약한 참조로 저장한다.
	LastAttacker = Attacker;
}

bool AHeroPlayerState::IsOnStreak()
{
	// 현재 이 Player가 Streak 상태인지 읽는다.
	return bOnStreak;
}

APlayerState* AHeroPlayerState::GetLastAttacker() const
{
	// Weak Pointer 대상이 아직 살아 있을 때만 raw 포인터를 반환한다.
	return LastAttacker.IsValid() ? LastAttacker.Get() : nullptr;
}

int32 AHeroPlayerState::GetScoredElims() const
{
	// Leader 계산에서 사용할 현재 누적 Elimination 수를 반환한다.
	return ScoredElims;
}

TArray<ESpecialElimType> AHeroPlayerState::DecodeElimBitmask(ESpecialElimType ElimTypeBitmask)
{
	// 입력 Bitmask에서 발견한 유효한 개별 Flag들을 순서대로 모으는 배열 
	TArray<ESpecialElimType> ValidElims;
	
	// Enum Wrapper를 벗겨 실제 16 bit 정수 값으로 변환
	uint16 BitmaskValue = static_cast<uint16>(ElimTypeBitmask);
	
	// uint16의 모든 Bit Position을 한 번씩 검사
	for (uint16 i = 0; i < 16; i++)
	{
		// 현재 i번째 Bit가 BitmaskValue와 동일한 위치에 켜져 있는가?_ 1 & 1 = 1 일때만 동작
		// 00000000 00000001 & 00000000 00000001 = 1 ( true )
		if (BitmaskValue & (1 << i))
		{
			// 검출된 Bit를 ESpecialElimType으로 복원
			ESpecialElimType EnumValue = static_cast<ESpecialElimType>(1 << i);
			// 요소를 Array 끝에 추가 ( 반환 순서가 UI 표시 순서가 될 수 있음 )
			ValidElims.Add(EnumValue);
		}
	}
	return ValidElims;
}




void AHeroPlayerState::Client_ScoredElim_Implementation(int32 ElimScore)
{
	// TODO:
	// 일반 Elimination 메시지와 새로운 Score를 HUD에 반영한다.
}

void AHeroPlayerState::Client_SpecialElim_Implementation(const ESpecialElimType& SpecialElim, int32 SequentialElimCount,
	int32 StreakCount, int32 ElimScore)
{
	// TODO:
	// SpecialElim Bitmask를 해석하고
	// 필요한 HUD 메시지와 Score를 갱신한다.
	
	// SpecialElimData가 반드시 유효해야 한다. but UE의 ensure는 실패되어도 계속 실행된다.
	ensure(IsValid(SpecialElimData));
	
	// 복합 Bitmask를 데이타에셋의 원자적 Map Key로 사용 가능
	TArray<ESpecialElimType> ElimTypes = DecodeElimBitmask(SpecialElim);
	// Decode 결과를 순회
	for (ESpecialElimType ElimType : ElimTypes)
	{
		// 원자적 Enum 하나를 Key로 사용 Massage, Icon, Runtime Field Container를 얻는다. (Lookup - 조회용)
		FSpecialElimInfo& ElimMessageInfo = SpecialElimData->SpecialElimInfo.FindChecked(ElimType);
		if (ElimType == ESpecialElimType::Sequential)
		{
			ElimMessageInfo.SequentialElimCount = SequentialElimCount;
		}
		if (ElimType == ESpecialElimType::Streak)
		{
			ElimMessageInfo.StreakCount = StreakCount;
		}
		// Struct에 ElimType도 저장
		// Queue에는 Map Key가 같이 저장되지 않는다. Elimtype 유형을 판단하려면 구조체 내부에 Type 필요
		// 큐에 들어가기 직전, 구조체 내부에 "내가 무슨 타입인지" 명시하여 독립성 확보
		ElimMessageInfo.ElimType = ElimType;
		
		// FIFO = First-in, First-out ( 대기실에 적재 )
		// Self Contained Payload 용 -> 데이터 자체만으로 모든 정보를 완벽하게 포함하고 있는 상태
		SpecialElimQueue.Enqueue(ElimMessageInfo);
	}
	if (!bIsProcessingQueue)
	{
		ProcessNextSpecialElim();
	}
}

void AHeroPlayerState::ProcessNextSpecialElim()
{
	// Dequeue()가 Queue의 가장 오래된 Item을 반환
	FSpecialElimInfo ElimInfo;
	if (SpecialElimQueue.Dequeue(ElimInfo))
	{
		bIsProcessingQueue = true;
		ShowSpecialElim(ElimInfo);
		
		// 현재 Dequeue 처리 전체를 확실히 끝낸 뒤 다음 Frame에서 실제 Delay Timer를 설정
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			FTimerHandle TimerHandle;	// 람다 지역 변수
			GetWorldTimerManager().SetTimer(
				TimerHandle, 
				this, 
				&AHeroPlayerState::ProcessNextSpecialElim, 
				ElimDisplayTime,
				false);
		});
	}
	else
	{
		// Queue가 비면 프로세싱 종료 ( Dequeue 실패시 )
		bIsProcessingQueue = false;
		
		// 이후 새로운 Client_SpecialElim RPC가 오면 다시 시작
	}
}

void AHeroPlayerState::ShowSpecialElim(const FSpecialElimInfo& ElimMessageInfo)
{
	FString ElimMessageString = ElimMessageInfo.ElimMessage;
	// Sequential 메시지 가공
	if (ElimMessageInfo.ElimType == ESpecialElimType::Sequential)
	{
		if (ElimMessageInfo.SequentialElimCount == 2) ElimMessageString = FString("Double Elim!");
		else if (ElimMessageInfo.SequentialElimCount == 3) ElimMessageString = FString("Triple Elim!");
		else if (ElimMessageInfo.SequentialElimCount == 4) ElimMessageString = FString("Quad Elim!");
		else if (ElimMessageInfo.SequentialElimCount > 5) 
			ElimMessageString = FString::Printf(TEXT("Rampage x%d!"), ElimMessageInfo.SequentialElimCount);	
	}
	// Streak 메시지 가공
	if (ElimMessageInfo.ElimType == ESpecialElimType::Streak) 
		ElimMessageString = FString::Printf(TEXT("Streak %d!"), ElimMessageInfo.StreakCount);
	
	// 위젯 생성
	if (IsValid(SpecialElimWidgetClass))
	{
		USpecialElim* ElimWidget = 
			CreateWidget<USpecialElim>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(ElimWidget))
		{
			ElimWidget->InitializeWidget(ElimMessageString, ElimMessageInfo.ElimIcon);
			ElimWidget->AddToViewport();
		}
	}
}

void AHeroPlayerState::Client_LostTheLeader_Implementation()
{
	// TODO: 클라이언트에게 선두 자리를 내주었다는 사실을 알립니다.
	ensure(IsValid(SpecialElimData));
	FSpecialElimInfo& ElimMessageInfo = SpecialElimData->SpecialElimInfo.FindChecked(ESpecialElimType::LostTheLead);
	
	if (IsValid(SpecialElimWidgetClass))
	{
		// 지역 변수 - 제거 후 더 이상 강한 참조가 없으면 나중에 GC 정리 대상이 될 수 있다. 단순 Popup에는 적절하다.
		USpecialElim* ElimWidget = 
			CreateWidget<USpecialElim>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(ElimWidget))
		{
			ElimWidget->InitializeWidget(ElimMessageInfo.ElimMessage, ElimMessageInfo.ElimIcon);
			ElimWidget->AddToViewport();
		}
	}
}
