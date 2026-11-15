# 인터페이스 명세서

- 프로젝트: Unreal Engine 5 기반 턴제 RPG 전투 시스템
- 작성자: 조의현
- 최초 작성일: 2026-11-15

이 문서는 전투 핵심 로직(DamageCalculator, SkillComponent, BattleManager, StatComponent, DataManager, FieldEncounter, UI)의
주요 함수 시그니처와 동작을 정리한다. 함수 시그니처·동작이 바뀌면 같은 커밋에서 이 문서도 갱신한다.

---

## 1. UDamageCalculator (RPGDamageCalculator.h/.cpp)

| 함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `CalculateFinalDamage` | `AActor* Attacker, AActor* Defender, const FSkillDataRow& SkillData` | `int32` | 최종 데미지 계산. `SkillData.ElementType`이 `Slash`/`Strike`면 `AtkStat.STR`, 그 외(원소 스킬)는 `AtkStat.MAG`로 공격력을 산출한다. 기본 데미지 `Max(1, Power + 공격력×0.5 − 방어 END×0.5)`, `Weak`/`Resist` 매칭(×1.5 / ×0.5, 둘 다면 약점만), 크리티컬(×1.5)을 순서대로 적용. 반환은 `Max(1, Round(데미지))`, 잘못된 입력이면 0. **2026-11-15 (5차, 테스트용 로그)**: 약점/내성/크리티컬이 적용될 때 각각 `[Damage] WEAK: …`, `[Damage] RESIST: …`, `[Damage] CRITICAL: …` 한 줄을 `LogTemp` Warning으로 남긴다(공격자·대상·스킬·배율, 크리티컬은 확률도). 시그니처·델리게이트 변경 없음. |
| `CheckEvasion` | `int32 AtkSPD, int32 DefSPD, int32 AtkLUK, int32 DefLUK` | `bool` | SPD/LUK 차이 기반 회피 판정. `Clamp(5 + (DefSPD−AtkSPD)×0.5 + (DefLUK−AtkLUK)×0.3, 0, 75)`을 0~100 롤과 비교. |
| `IsElementListed` | `const FString& CommaList, FName ElementType` | `bool` | 쉼표로 구분된 속성 목록(Weak/Resist)에 `ElementType`이 포함되는지 확인. |

**2026-11-15 변경**: 크리티컬 확률을 `치명률(%) = BaseCrit × 100 + LUK × 0.1`로 수정 (`BaseCrit`은 CSV 비율 0.05 = 5%). STR/MAG 분기를 `Slash`/`Strike` 기준으로 수정(이전엔 CSV에 없는 `"Physical"`을 비교해 전부 MAG로 계산되던 버그).

---

## 2. URPGSkillComponent (RPGSkillcomponent.h/.cpp)

| 함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `ExecuteSkill` | `AActor* Target, FName SkillID` | `bool` | 실행 순서: `AvailableSkills` 포함 확인 → 데이터 조회(몬스터는 SK_MONSTER, 플레이어는 SK_DATA) → 해금 확인(`IsSkillUnlocked`, 플레이어만) → **비용 확인(`CanAffordSkill`, 플레이어만)** → 비용 차감 → `CheckEvasion` → `CalculateFinalDamage` → `TakeRPGDamage` → (대상이 몬스터면) `AddAggro` → `OnSkillResolved` 브로드캐스트. 회피되면 `OnSkillResolved(Target, 0, true)` 후 true. |
| `CanAffordSkill` | `FName SkillID` | `bool` | **(신규, 2026-11-15)** 비용 충분 판정의 단일 기준. 몬스터 소유자면 항상 true. SK_DATA에서 행을 못 찾으면 false(fail-closed). `CostType == "SP"`: `CurrentSP >= GetSkillCost`. `CostType == "HP"`: `CurrentHP > GetSkillCost` — **같으면 부족**(사용 시 시전자가 쓰러짐). `None`/그 외: true. |
| `GetSkillCost` | `const FSkillDataRow& SkillData` | `int32` | **(신규, 2026-11-15, 4차)** 스킬의 실제 소모량. `SP` → `RoundToInt(CostValue)`, `HP` → `RoundToInt(CostValue × 소유자 MonsterStat.MaxHP)`, `None`/그 외 → 0. `CanAffordSkill`(판정), `ExecuteSkill`(차감), `UCommandMenu::RebuildSkillList`(라벨 "HP 16")가 모두 이 함수를 쓴다. |

**2026-11-15 변경 (FR-07)**: `ExecuteSkill`의 비용 검사가 `CanAffordSkill`을 호출하도록 통일. 이전엔 HP를 `CurrentHP < HPCost`로 검사해 HP와 비용이 같을 때 실행이 허용됐다. 비용 차감 로직은 그대로.
**2026-11-15 변경 (4차)**: 두 곳에 중복돼 있던 비용 계산식을 `GetSkillCost`로 묶음. 판정·차감 결과는 동일.

---

## 3. URPGStatComponent (RPGStatComponent.h/.cpp)

| 함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `InitializeStat` | `const FMonsterDataRow& InData` | `void` | `BaseStat`/`MonsterStat`을 `InData`로 초기화하고 `CurrentHP`/`CurrentSP`를 Max로 채운다. |
| `ApplyCoreBonus` | `const FCoreDataRow& CoreData` | `void` | `BaseStat`에 `CoreData`의 Bonus 스탯(HP/SP/STR/MAG/END/SPD/LUK)을 더해 `MonsterStat`을 재계산. 코어 재장착 시에도 `BaseStat`부터 다시 계산하므로 보너스가 중첩되지 않는다. `CoreData.Weak`/`Resist`가 비어 있지 않고 `"None"`이 아니면 `MonsterStat`의 값을 코어 값으로 덮어쓴다(FR-10). |
| `ApplyGrowth` | `const FGrowthDataRow& GrowthData` | `void` | `BaseStat`을 레벨의 절대 스탯으로 교체 후 `ActiveCoreBonus`를 재적용하고, 늘어난 MaxHP/MaxSP만큼 `CurrentHP`/`CurrentSP`를 더한다(풀힐 아님). |

---

## 4. ARPGBattleManager (RPGBattleManager.h/.cpp)

| 멤버/함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `bIsBossBattle` *(UPROPERTY)* | — | `bool` | 현재 전투가 보스전인지. `BeginFieldEncounter`가 인카운터의 `bIsBossEncounter`에서 복사한다(일반 인카운터면 false). |
| `BossStageNumber` *(UPROPERTY)* | — | `int32` | 보스전 승리 시 저장할 스테이지 번호. `BeginFieldEncounter`가 인카운터의 `BossStageNumber`에서 복사(일반이면 0). |
| `bAutoStartWithPlacedMonsters` *(UPROPERTY)* | — | `bool` | **(신규, 2026-11-15)** 기본 false. true일 때만 `BeginPlay`가 레벨에 배치된 `ARPGMonsterBase`를 모아 즉시 `StartBattle`한다(그레이박스 테스트용). 실제 레벨은 false로 두어 모든 전투가 `AFieldEncounter`에서 시작되게 한다. |
| `StartBattle` | `const TArray<ARPGMonsterBase*>& Enemies` | `void` | 월드의 모든 `APlayerCharacter`를 다시 수집해 `PlayerParty`로, `Enemies`를 `EnemyParty`로 설정하고 전투 시작. |
| `BeginFieldEncounter` | `FName StageID, FName BattleID, AFieldEncounter* SourceEncounter` | `void` | `ENC_DATA`에서 `StageID`/`BattleID`로 조회, 몬스터 스폰 후 아레나로 전환. `SourceEncounter`를 `PendingSourceEncounter`에 보관하고, **`bIsBossBattle`/`BossStageNumber`를 인카운터 값으로 설정**한다. |
| `IsTutorialBattle` | — | `bool` | `PendingSourceEncounter`가 있고 그 `StageID == "Tutorial"`이면 true. `HandleVictory`의 튜토리얼 클리어 저장과 `UBattleHUD`의 안내 텍스트 표시에 쓰인다. |
| `SubmitAction` | `ARPGCharacterBase* Actor, FName SkillID, AActor* Target` | `void` | 선택된 스킬을 실행하고, 성공 여부와 관계없이 `AdvanceTurn`을 호출한다. |
| `BuildTurnQueue` *(private)* | — | `void` | 생존자를 SPD 내림차순으로 **`StableSort`** 정렬(SPD가 같으면 수집 순서 = 아군 먼저 유지). 라운드가 끝나면 `State_CalculateTurn`으로 돌아가 재계산. |
| `StartCurrentActorTurn` *(private)* | — | `void` | 아군 차례면 `UGameplayStatics::GetPlayerController(GetWorld(), 0)`을 `ARPGPlayerController`로 캐스팅해 `BeginSkillSelection`을 호출한다(레벨 배치 동료도 같은 컨트롤러가 명령, FR-06). 못 찾으면 로그 후 `AdvanceTurn`. |
| `HandleVictory` *(private)* | — | `void` | 생존 아군에 EXP 지급 → 적 제거 → **매 승리마다 `SaveProgress(플레이어 폰의 EquippedCoreID, 0)`** (FR-15, ClearedStage는 기존 Max 보정으로 유지) → 보스전이면 `HandleStageCleared(BossStageNumber)` → 튜토리얼 전투면 `SaveTutorialCleared()` → `ReturnPartyToField()`. 튜토리얼 판별은 `ReturnPartyToField`가 `PendingSourceEncounter`를 비우기 전에 한다. |
| `HandleStageCleared` *(private)* | `int32 ClearedStageNumber` | `void` | `SaveProgress(플레이어 폰의 EquippedCoreID, ClearedStageNumber)`. 이어지는 동료 합류(`SpawnCompanion`)는 확장 기능이며, PC_002가 이미 파티에 있으면 아무 일도 하지 않는다. |
| `CheckBattleEnd` *(private)* | — | `bool` | 아군 또는 적이 전멸하면 승리 시 `HandleVictory()` 후 **`SetBattleState(State_BattleEnd)`**, 패배 시 `State_BattleEnd`만 설정하고 true. `HandleVictory`는 필드 복귀를 타이머로 예약만 하므로 `State_BattleEnd` 설정이 복귀보다 먼저 일어난다. |
| `ReturnPartyToField` / `FinishReturnToField` *(private)* | — | `void` | 승리 시에만 호출. `ReturnPartyToField`가 페이드 아웃 후 `FadeDuration` 타이머로 `FinishReturnToField`를 예약. `FinishReturnToField`는 파티를 필드 위치로 되돌리고 인카운터를 제거, 페이드 인·입력 복구. **2026-11-15 (5차, FR-02/FR-17)**: 끝에 `TurnQueue.Reset()`, `CurrentTurnIndex = 0`, `OnTurnQueueChanged.Broadcast()` 후 **`SetBattleState(State_Wait)`**. 이전엔 복귀 후에도 `State_BattleEnd`가 남아 VICTORY 문구와 턴 순서 UI가 필드에 계속 보였다. 패배는 이 경로를 타지 않으므로 `State_BattleEnd` 유지(게임오버 UI는 범위 밖). |
| `GetPlayerPawnEquippedCoreID` *(private)* | — | `FName` | **(신규, 2026-11-15)** `UGameplayStatics::GetPlayerPawn(GetWorld(), 0)`을 `APlayerCharacter`로 캐스팅해 `EquippedCoreID`를 반환. `PlayerParty[0]`은 순서가 보장되지 않아 코어 없는 동료가 저장될 수 있으므로 쓰지 않는다. |
| `GetCombatantDisplayName` | `const ARPGCharacterBase* Combatant` | `FString` | **(신규, 2026-11-15, 4차, FR-16)** UI에 표시할 이름. `StatComponent->MonsterStat.MonsterName`(아군은 PC_DATA `Name`, 적은 MD_DATA `MonsterName`)을 쓴다. 적은 `EnemyParty`에 같은 `MonsterID`가 2마리 이상이면 `EnemyParty` 순서로 `" 1"`, `" 2"`를 붙인다(죽은 개체도 번호 유지). 스탯이 없거나 이름이 `None`이면 `GetName()`으로 폴백. 커맨드 메뉴·턴 순서·HUD가 모두 이 함수를 써서 번호가 일치한다. |

**2026-11-15 변경**:
- `StartCurrentActorTurn`: `Player->GetController()` → `GetPlayerController(0)` (동료 턴 스킵 버그 수정).
- `IsTutorialBattle()` 신설, `HandleVictory`에서 튜토리얼 클리어 저장.
- `HandleVictory`: 매 승리마다 장착 코어 저장 추가 (FR-15 — 코어를 바꾼 뒤 종료해도 복원되게).
- `HandleStageCleared`: 저장 코어를 `PlayerParty[0]` → 플레이어 폰 기준으로 (`GetPlayerPawnEquippedCoreID`).
- `BeginFieldEncounter`: 인카운터의 보스 값을 `bIsBossBattle`/`BossStageNumber`로 복사 (FR-18).
- `BeginPlay`: 자동 전투 시작을 `bAutoStartWithPlacedMonsters`로 막음.
- `BuildTurnQueue`: `Sort` → `StableSort`.

---

## 5. AFieldEncounter (FieldEncounter.h/.cpp)

| 멤버/함수 | 설명 |
|---|---|
| `StageID`, `BattleID` *(UPROPERTY)* | ENC_DATA 조회 키. 플레이어 캐릭터가 트리거에 닿으면 `GetActorOfClass`로 찾은 `ARPGBattleManager`의 `BeginFieldEncounter(StageID, BattleID, this)`를 호출한다. |
| `bIsBossEncounter` *(UPROPERTY, EditAnywhere)* | **(신규, 2026-11-15)** 기본 false. true면 이 인카운터로 시작한 전투가 보스전으로 취급된다. |
| `BossStageNumber` *(UPROPERTY, EditAnywhere, EditCondition = bIsBossEncounter)* | **(신규, 2026-11-15)** 보스전 승리 시 저장할 스테이지 번호. 레벨의 보스 인카운터는 `Stage1`/`Boss`, `bIsBossEncounter = true`, `BossStageNumber = 1`. |

레벨에는 `ARPGBattleManager`를 **하나만** 둔다. `AFieldEncounter`와 `ARPGPlayerController`가 `GetActorOfClass`로 찾기 때문에 둘 이상이면 어느 쪽에 연결될지 보장되지 않는다.

---

## 6. URPGDataManager (RPGDataManager.h/.cpp)

| 함수 | 파라미터 | 반환값 | 설명 |
|---|---|---|---|
| `Initialize` *(override)* | `FSubsystemCollectionBase& Collection` | `void` | **(신규, 2026-11-15, FR-15)** `Super::Initialize` 뒤에 `LoadProgress()`를 호출한다. 세이브가 없으면 `CurrentSaveGame`은 null로 남고 새 게임(ClearedStage 0, 튜토리얼 미클리어)으로 시작한다. |
| `SaveProgress` | `FName EquippedCoreID, int32 ClearedStage` | `void` | 새 `URPGSaveGame`을 만들어 저장. `ClearedStage = Max(새 값, 기존 값)`로 줄어들지 않고, `bTutorialCleared`는 기존 `CurrentSaveGame`에서 이어받는다. 세이브 슬롯 `"RPGSaveSlot"`. |
| `SaveTutorialCleared` | — | `void` | 새 `URPGSaveGame`을 만들어 `bTutorialCleared = true`로 저장. `EquippedCoreID`/`ClearedStage`는 기존 `CurrentSaveGame`에서 이어받는다. |
| `LoadProgress` | — | `bool` | 저장 슬롯에서 `CurrentSaveGame`을 불러온다. 슬롯이 없으면 false. |
| `IsSkillUnlocked` | `FName SkillID` | `bool` | SK_DATA의 `Quest` 값을 `ClearedStage`/`bTutorialCleared`로 판정. |
| `IsCoreUnlocked` | `FName CoreID` | `bool` | CD_DATA의 `Quest` 값을 `ClearedStage`/`bTutorialCleared`로 판정. |
| `IsQuestConditionMet` *(내부 헬퍼, .cpp 익명 네임스페이스)* | `FName Quest, int32 ClearedStage, bool bTutorialCleared` | `bool` | `"None"` → true, `"TUTORIAL_CLEAR"` → `bTutorialCleared`, `"STAGE<N>_CLEAR"` → `ClearedStage >= N`. 인식 못 하는 값은 false(fail-closed). |

---

## 7. APlayerCharacter (PlayerCharacter.h/.cpp)

| 함수 | 설명 |
|---|---|
| `InitializeCharacterStat` | PC_DATA를 `StatComponent`에 적용. `EquippedCore`가 있는 캐릭터(주인공)는 `ChangeCore`로 코어 장착, 없으면(동료) `SkillPool`을 그대로 `AvailableSkills`에 넣는다. **2026-11-15 (FR-15)**: 코어 장착 분기에서 `CurrentSaveGame->EquippedCoreID`가 있고 `IsCoreUnlocked`가 true면 PC_DATA 기본 코어 대신 저장된 코어를 장착한다. 동료는 이 분기에 들어오지 않으므로 세이브 코어가 동료에게 번지지 않는다. |
| `ChangeCore` | `FName CoreID` → `void`. 해금 확인 → `GetCoreData` → `ApplyCoreBonus` → 코어 스킬로 `AvailableSkills` 교체. 실패해도 반환값이 없어 호출자(`RequestChangeCore`)는 성공 여부를 알 수 없다(현행 유지, 과제3 U3 의사코드와 일치). |

---

## 8. UCommandMenu / UTurnQueueUI / UBattleHUD (CommandMenu.cpp, TurnQueueUI.cpp, BattleHUD.h/.cpp)

| 멤버/함수 | 설명 |
|---|---|
| `UCommandMenu::RebuildSkillList` *(private)* | 행동 캐릭터의 `AvailableSkills`로 버튼을 만든다. **2026-11-15 (FR-07)**: 버튼 활성 조건을 `IsSkillUnlocked && CanAffordSkill`로 변경. 비용 부족 스킬은 `SetIsEnabled(false)`만 하고 라벨은 비용 때문에 바꾸지 않는다. **2026-11-15 (4차, NFR-08)**: 라벨을 SK_DATA 기준 `"스킬이름 [속성] SP 10"` 형식으로. `CostType`이 `SP`/`HP`일 때만 `" <CostType> <GetSkillCost>"`를 붙이고(HP는 실제 차감량, 예 `HP 16`), `None`이면 비용 생략. 행이 없으면 ID 표시. 잠긴 스킬은 `" (잠김)"` 접미. |
| `UCommandMenu::RebuildTargetList` *(private)* | `PossibleTargets`로 버튼을 만든다. **2026-11-15 (4차, NFR-08)**: 대상이 `ARPGCharacterBase`고 `StatComponent`가 있으면 라벨을 `"<GetCombatantDisplayName> HP <CurrentHP>/<MaxHP> 약점:<Weak> 내성:<Resist>"`로. `Weak`/`Resist`는 MD_DATA 문자열 그대로(예 `Ice, Strike`), 비어 있거나 `None`이면 `없음`. 배틀 매니저는 `BoundController->GetActiveBattleManager()`로 얻고, 없으면 번호 없는 `MonsterName`. 그 외 액터는 `GetName()`. |
| `UCommandMenu::RebuildCoreList` *(private)* | `CoreListContainer`에 모든 코어 버튼을 만든다. 이번 턴에 이미 교체했으면(`bHasChangedCoreThisTurn`) 비워 둔다. **2026-11-15 (FR-10)**: 행동 캐릭터의 `EquippedCoreID`가 `None`(동료, PC_002)이면 목록을 만들지 않고 비워 둔다. 코어 교체는 기본 코어가 있는 캐릭터(PC_001)만 가능. **2026-11-15 (4차, NFR-08)**: 라벨을 CD_DATA `CoreName`으로(없으면 ID), 잠긴 코어는 `"코어이름 (잠김)"` 비활성. |
| *(CommandMenu.cpp 파일 정적)* `LockedSuffix`, `NoneAffinity`, `FormatAffinityForLabel` | **(신규, 2026-11-15, 4차)** 한글 UI 문구(`" (잠김)"`, `"없음"`, `"약점:"`, `"내성:"`)는 소스 파일을 ASCII로 유지하기 위해 `\u` 이스케이프로 쓰고 옆에 영어 주석을 단다. `FormatAffinityForLabel`은 Weak/Resist 문자열을 trim하고 빈 값/`None`을 `없음`으로 바꾼다. |
| `UTurnQueueUI::RefreshQueue` | 턴 큐를 한 줄씩 표시(현재 턴은 `"-> "` 접두). **2026-11-15 (4차, FR-16)**: `Combatant->GetName()` 대신 `BoundBattleManager->GetCombatantDisplayName`을 쓴다(아군 `주인공`/`동료1`, 적 `훈련용 슬라임 1`). 바인딩된 매니저가 없이 호출되면 기존 `GetName()`. |
| `UBattleHUD::RefreshStatusText` *(private)* | `PartyStatusText`/`EnemyStatusText`에 `"<이름> HP: cur/max"` 줄을 쌓는다. **2026-11-15 (4차, FR-16)**: 이름을 `GetCombatantDisplayName`으로 통일. |
| `UBattleHUD::TutorialText` *(UPROPERTY, BindWidgetOptional)* | 튜토리얼 안내 텍스트 위젯. 위젯 블루프린트에 없어도 빌드·실행이 깨지지 않는다. |
| `UBattleHUD::TutorialMessage` *(UPROPERTY, EditAnywhere FText)* | `TutorialText`에 표시할 문구. 위젯 블루프린트에서 입력(C++ 하드코딩 없음). |
| `UBattleHUD::InitializeForBattle` | 게임 시작 시 `ARPGPlayerController::BeginPlay`에서 **한 번만** 호출된다. 델리게이트 바인딩 후 `TutorialText`를 Collapsed로 초기화만 한다. |
| `UBattleHUD::HandleBattleStateChanged` | `OnBattleStateChanged` 수신. `BattleResultText`는 `State_BattleEnd`에서 VICTORY/DEFEAT 표시, 그 외 상태(`State_Wait` 포함)는 Collapsed. **2026-11-15 (5차, FR-02/FR-17)**: `TurnQueueWidget`을 `State_Wait`에서 Collapsed, 그 외 상태에서 Visible로 전환한다(승리 후 필드 복귀 시 `FinishReturnToField`가 `State_Wait`로 바꾸면서 숨겨지고, 다음 전투의 `State_CalculateTurn`에서 다시 보임). **2026-11-15 (NFR-08)**: `TutorialText`는 전투 중 상태(`State_CalculateTurn`/`State_PlayerTurn`/`State_EnemyTurn`)이고 `BoundBattleManager->IsTutorialBattle()`이면 `TutorialMessage`를 넣고 Visible, 그 외(`State_Wait`/`State_BattleEnd`)는 Collapsed. (처음엔 `InitializeForBattle`에서 판정했는데, 그 시점은 전투 전이라 항상 false여서 여기로 옮김.) |

---

## 9. ARPGPlayerController (RPGPlayerController.h/.cpp)

| 함수 | 매개변수 | 반환 | 설명 |
|---|---|---|---|
| `RequestChangeCore` | `FName NewCoreID` | `void` | 커맨드 메뉴의 코어 버튼(`OnCoreChosen`)에서 호출. 턴당 1회(`bHasChangedCoreThisTurn`), 턴을 소모하지 않는다. **2026-11-15 (FR-10)**: 행동 캐릭터(`ActingCharacter`)의 `EquippedCoreID`가 `None`이면 경고 로그만 남기고 반환한다. `ChangeCore`를 호출하지 않고 `bHasChangedCoreThisTurn`도 그대로 둔다(교체 횟수 미소모). 성공 시 `ChangeCore` → `bHasChangedCoreThisTurn = true` → `ShowCommandMenu`로 메뉴 재생성. |
| `GetActiveBattleManager` | — | `ARPGBattleManager*` | **(신규, 2026-11-15, 4차)** `BlueprintPure` 인라인 getter. private `ActiveBattleManager`를 UI(`UCommandMenu::RebuildTargetList`)가 `GetCombatantDisplayName` 호출에 쓸 수 있게 공개한다. 내 턴이 아니면 null. |
