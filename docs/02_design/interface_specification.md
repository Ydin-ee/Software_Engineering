# 인터페이스 명세서

- 프로젝트: Unreal Engine 5 기반 턴제 RPG 전투 시스템
- 작성자: 조의현
- 최초 작성일: 2026-11-15

이 문서는 전투 핵심 로직(DamageCalculator, BattleManager, StatComponent, DataManager)의
주요 함수 시그니처와 동작을 정리한다. 함수 시그니처·동작이 바뀌면 같은 커밋에서 이 문서도 갱신한다.

---

## 1. UDamageCalculator (RPGDamageCalculator.h/.cpp)

| 함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `CalculateFinalDamage` | `AActor* Attacker, AActor* Defender, const FSkillDataRow& SkillData` | `int32` | 최종 데미지 계산. `SkillData.ElementType`이 `Slash`/`Strike`면 `AtkStat.STR`, 그 외(원소 스킬)는 `AtkStat.MAG`로 공격력을 산출한다. `DefStat.END`로 방어, `Weak`/`Resist` 매칭(1.5배/0.5배), 크리티컬(1.5배)을 순서대로 적용한다. |
| `CheckEvasion` | `int32 AtkSPD, int32 DefSPD, int32 AtkLUK, int32 DefLUK` | `bool` | SPD/LUK 차이 기반 회피 판정. `5 + (DefSPD-AtkSPD)*0.5 + (DefLUK-AtkLUK)*0.3`을 0~75 범위로 Clamp 후 0~100 롤과 비교. |
| `IsElementListed` | `const FString& CommaList, FName ElementType` | `bool` | 쉼표로 구분된 속성 목록(Weak/Resist)에 `ElementType`이 포함되는지 확인. |

**2026-11-15 변경**: `CalculateFinalDamage`의 크리티컬 확률 계산에서 `SkillData.BaseCrit`(CSV 비율, 0.05=5%)을 **×100**하여 `AtkStat.LUK*0.1`과 더한 뒤 0~100 롤과 비교하도록 수정. (§2. 데이터 명세서 참고)

---

## 2. URPGStatComponent (RPGStatComponent.h/.cpp)

| 함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `InitializeStat` | `const FMonsterDataRow& InData` | `void` | `BaseStat`/`MonsterStat`을 `InData`로 초기화하고 `CurrentHP`/`CurrentSP`를 Max로 채운다. |
| `ApplyCoreBonus` | `const FCoreDataRow& CoreData` | `void` | `BaseStat`에 `CoreData`의 Bonus 스탯(HP/SP/STR/MAG/END/SPD/LUK)을 더해 `MonsterStat`을 재계산. **코어 재장착 시에도 `BaseStat`부터 다시 계산**하므로 보너스가 중첩되지 않는다. |
| `ApplyGrowth` | `const FGrowthDataRow& GrowthData` | `void` | `BaseStat`을 레벨의 절대 스탯으로 교체 후 `ActiveCoreBonus`를 재적용하고, 늘어난 MaxHP/MaxSP만큼 `CurrentHP`/`CurrentSP`를 더한다(풀힐 아님). |

**2026-11-15 변경**: `ApplyCoreBonus`가 `MonsterStat.Weak`/`MonsterStat.Resist`를 `CoreData.Weak`/`CoreData.Resist`로 덮어쓰도록 수정. `CoreData`의 값이 비어 있거나 `"None"`이면 `BaseStat`의 값(캐릭터 기본 약점/내성)을 그대로 유지한다. (§2. 데이터 명세서 참고)

---

## 3. ARPGBattleManager (RPGBattleManager.h/.cpp)

| 함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `StartBattle` | `const TArray<ARPGMonsterBase*>& Enemies` | `void` | 월드의 모든 `APlayerCharacter`를 다시 수집해 `PlayerParty`로, `Enemies`를 `EnemyParty`로 설정하고 전투 시작. |
| `BeginFieldEncounter` | `FName StageID, FName BattleID, AFieldEncounter* SourceEncounter` | `void` | `ENC_DATA`에서 `StageID`/`BattleID`로 조회, 몬스터 스폰 후 아레나로 전환. `SourceEncounter`를 `PendingSourceEncounter`에 보관한다. |
| `IsTutorialBattle` | (없음) | `bool` | **(신규)** `PendingSourceEncounter`가 있고 그 `StageID == "Tutorial"`이면 true. `HandleVictory`의 튜토리얼 클리어 저장과 `UBattleHUD`의 안내 텍스트 표시 여부에 쓰인다. |
| `SubmitAction` | `ARPGCharacterBase* Actor, FName SkillID, AActor* Target` | `void` | 선택된 스킬을 실행하고 턴을 넘긴다. |
| `StartCurrentActorTurn` *(private)* | (없음) | `void` | 현재 턴 캐릭터가 플레이어 파티면 `UCommandMenu`를 띄운다. |

**2026-11-15 변경**: `StartCurrentActorTurn`에서 턴 캐릭터 자신의 컨트롤러(`Player->GetController()`) 대신 **`UGameplayStatics::GetPlayerController(GetWorld(), 0)`** 하나로 명령하도록 수정. 레벨에 배치된 동료(PC_002)는 자기 컨트롤러가 없어 턴이 스킵되던 버그 수정.

**2026-11-15 추가**: `IsTutorialBattle()` 신설. `HandleVictory()`가 `ReturnPartyToField()`(= `PendingSourceEncounter`를 정리) 호출 **이전에** `IsTutorialBattle()`을 확인해 `URPGDataManager::SaveTutorialCleared()`를 호출한다.

---

## 4. URPGDataManager (RPGDataManager.h/.cpp)

| 함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `SaveProgress` | `FName EquippedCoreID, int32 ClearedStage` | `void` | 새 `URPGSaveGame`을 만들어 저장. `ClearedStage`는 기존 값보다 낮게 내려가지 않는다. |
| `SaveTutorialCleared` | (없음) | `void` | **(신규)** 새 `URPGSaveGame`을 만들어 `bTutorialCleared = true`로 저장. `EquippedCoreID`/`ClearedStage`는 기존 `CurrentSaveGame`에서 이어받는다. |
| `LoadProgress` | (없음) | `bool` | 저장 슬롯에서 `CurrentSaveGame`을 불러온다. |
| `IsSkillUnlocked` | `FName SkillID` | `bool` | `SK_DATA`의 `Quest` 값을 `ClearedStage`/`bTutorialCleared`로 판정. |
| `IsCoreUnlocked` | `FName CoreID` | `bool` | `CD_DATA`의 `Quest` 값을 `ClearedStage`/`bTutorialCleared`로 판정. |
| `IsQuestConditionMet` *(내부 헬퍼, .cpp 익명 네임스페이스)* | `FName Quest, int32 ClearedStage, bool bTutorialCleared` | `bool` | `Quest` 문자열이 `"None"`이면 true, `"TUTORIAL_CLEAR"`면 `bTutorialCleared` 그대로 반환, `"STAGE<N>_CLEAR"`면 `ClearedStage >= N`. 인식 못 하는 값은 false(fail-closed). |

**2026-11-15 변경**:
- `SaveProgress`가 `bTutorialCleared`를 `CurrentSaveGame`에서 이어받도록 수정 (이전에는 매 호출마다 false로 덮어써짐 — 보스전 저장 시 튜토리얼 클리어 기록이 날아가는 버그였음).
- `IsQuestConditionMet` 시그니처에 `bool bTutorialCleared` 파라미터 추가, `"TUTORIAL_CLEAR"` 문자열 인식 추가. `IsSkillUnlocked`/`IsCoreUnlocked`가 `CurrentSaveGame->bTutorialCleared`를 전달하도록 수정.
- `SaveTutorialCleared()` 신설.

---

## 5. UBattleHUD (BattleHUD.h/.cpp)

| 멤버/함수 | 설명 |
|---|---|
| `TutorialText` *(UPROPERTY, BindWidgetOptional)* | **(신규)** 튜토리얼 전투일 때만 표시되는 안내 텍스트 위젯. 위젯 블루프린트에 아직 없어도 빌드는 깨지지 않는다(`BindWidgetOptional`). |
| `TutorialMessage` *(UPROPERTY, EditAnywhere FText)* | **(신규)** `TutorialText`에 표시할 문구. C++에 하드코딩하지 않고 위젯 블루프린트에서 입력. |
| `InitializeForBattle` | `InBattleManager->IsTutorialBattle()`이 true면 `TutorialText`에 `TutorialMessage`를 표시(Visible), 아니면 Collapsed로 숨긴다. |
