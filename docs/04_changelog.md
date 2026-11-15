# 변경 이력

- 프로젝트: Unreal Engine 5 기반 턴제 RPG 전투 시스템
- 작성자: 조의현

날짜는 커밋 날짜 기준으로 기록한다.

---

## 2026-11-15 (5차) — 승리 후 필드 복귀 시 전투 UI 잔류 버그, 데미지 로그

### 버그 수정

- **승리 후 필드에서 VICTORY 문구·턴 순서 UI 잔류 (FR-02, FR-17)** — `RPGBattleManager.cpp`, `BattleHUD.cpp`
  - 원인(코드로 확인): `CheckBattleEnd`가 `HandleVictory()`(필드 복귀를 `FadeDuration` 타이머로 예약만 함) 직후 `SetBattleState(State_BattleEnd)`를 호출하고, 타이머로 실행되는 `FinishReturnToField`는 상태를 바꾸지 않아 복귀 후에도 `CurrentState == State_BattleEnd`로 남음. `TurnQueue`도 다음 `StartBattle`까지 비워지지 않아 턴 순서 UI가 마지막 큐(제거된 몬스터 포함)를 계속 표시.
  - 결정: 승리 후 필드 복귀가 끝나면 `State_Wait`. 패배는 지금처럼 `State_BattleEnd` 유지(게임오버 UI는 범위 밖).
  - `ARPGBattleManager::FinishReturnToField`: 끝에 `TurnQueue.Reset()`, `CurrentTurnIndex = 0`, `OnTurnQueueChanged.Broadcast()`(턴 순서 UI 비움) 후 `SetBattleState(State_Wait)`. 이 함수는 승리 경로에서만 호출된다.
  - `UBattleHUD::HandleBattleStateChanged`: `TurnQueueWidget`을 `State_Wait`에서 Collapsed, 그 외 상태에서 Visible. `BattleResultText`는 기존 로직대로 `State_BattleEnd` 외에는 Collapsed라 추가 수정 없음.

### 테스트용 로그 추가

- `UDamageCalculator::CalculateFinalDamage` — 약점 적용 시 `[Damage] WEAK: <공격자> hit <대상> with <스킬> (<속성>) x1.5`, 내성 적용 시 `[Damage] RESIST: … x0.5`, 크리티컬 시 `[Damage] CRITICAL: … x1.5 (chance N%)`. 기존 `landed a critical hit!` 문구를 `CRITICAL:` 형식으로 교체. 시그니처·델리게이트 변경 없음.

### 영향받는 산출 문서

- `02_design/interface_specification.md` — §1 `CalculateFinalDamage` 로그, §4 `CheckBattleEnd`·`ReturnPartyToField`/`FinishReturnToField` 행 추가, §8 `HandleBattleStateChanged`.
- `03_test.md` — TC-03 로그 문구 갱신, TC-19(승리 후 필드 복귀 시 결과 문구·턴 순서 숨김, FR-02/FR-17), TC-19a(패배 시 DEFEAT 유지) 추가.
- 시퀀스 다이어그램: `FinishReturnToField` → `OnTurnQueueChanged` 브로드캐스트, `SetBattleState(State_Wait)` 메시지 추가.

---

## 2026-11-15 (4차) — 전투 UI 표시 문구 개선 (FR-16, NFR-08)

### 기능 개선

- **문제**: 커맨드 메뉴와 턴 순서 UI가 ID·액터 이름(`SK_002`, `COR_001`, `RPGMonsterBase_0`, `BP_ThirdPersonCharacter_C_UAID…`)만 보여 줘서 플레이어가 스킬 속성·비용과 적의 약점을 알 수 없었음. NFR-08(약점 공략 안내), FR-16(턴 순서 표시) 근거.
- **결정**: 표시 문구는 전부 DataTable 값(SK/MD/PC/CD_DATA)으로 만들고 수치는 하드코딩하지 않는다. 이전 T5의 "라벨 바꾸지 마"는 비용 부족 판정에 한한 것이고, 이번엔 의도적으로 라벨 형식을 바꾼다. 기존 함수·변수 이름은 유지, 아래 3개 함수만 신규.
- **스킬 버튼** — `CommandMenu.cpp` `RebuildSkillList`: `"스킬이름 [속성] SP 10"`. 비용 `None`이면 비용 생략, HP 비용은 실제 차감량(`"HP 16"`). 잠김은 `" (잠김)"`.
- **대상 버튼** — `CommandMenu.cpp` `RebuildTargetList`: `"몬스터이름 HP 64/80 약점:Fire 내성:Ice"`. 같은 몬스터가 여러 마리면 이름 뒤 번호(`훈련용 슬라임 1`, `2`). Weak/Resist 비어 있거나 `None`이면 `없음`.
- **코어 버튼** — `CommandMenu.cpp` `RebuildCoreList`: CD_DATA `CoreName`, 잠김은 `"코어이름 (잠김)"`. 스킬과 통일해 `(Locked)` → `(잠김)`.
- **턴 순서 UI** — `TurnQueueUI.cpp` `RefreshQueue`: 아군은 PC_DATA `Name`(주인공/동료1), 적은 대상 버튼과 같은 이름·번호.
- **HUD 상태 텍스트** — `BattleHUD.cpp` `RefreshStatusText`: 아군/적 HP 줄도 같은 이름 헬퍼로 통일(FR-16).
- **신규 함수**
  - `URPGSkillComponent::GetSkillCost(const FSkillDataRow&) const` — `CanAffordSkill`/`ExecuteSkill`에 중복돼 있던 비용식을 묶고 라벨도 같은 값을 쓰게 함. 판정·차감 결과 동일.
  - `ARPGBattleManager::GetCombatantDisplayName(const ARPGCharacterBase*) const` — `MonsterStat.MonsterName` + 같은 `MonsterID`가 2마리 이상일 때 `EnemyParty` 순서 번호. 세 UI가 공유해 번호가 일치.
  - `ARPGPlayerController::GetActiveBattleManager() const` — private `ActiveBattleManager`의 읽기용 getter.
- **한글 문구 처리**: `.cpp`가 ASCII라 `(잠김)`/`약점:`/`내성:`/`없음`은 `\u` 이스케이프로 쓰고 옆에 영어 주석(`// " (Locked)"` 등)을 달았다.

### 데이터 변경 — 보스 밸런스 조정 (NFR-06 전체 클리어 가능하도록)

- 시험 조정. 2인 파티(PC_001 + PC_002)가 Stage1 보스를 끝까지 클리어할 수 있도록 보스 체력과 보스 스킬 위력을 낮췄다. 10회 전체 플레이(TC-14) 결과를 보고 확정한다.
- `docs/data/MD_DATA.csv` — `MOB_003`(스테이지1 보스) `MaxHP` **600 → 350**. 그 외 컬럼 변경 없음.
- `docs/data/SK_MONSTER.csv` — `SK_B02`(무자비한 분쇄, Strike) `Power` **65 → 35**. 그 외 컬럼 변경 없음.
- `GameData.xlsx`도 같이 수정됨. 에디터에서 `MD_DATA`, `SK_MONSTER` DataTable **Reimport** 필요.
- `02_design/data_specification.md`에는 두 값이 적혀 있지 않아(ENC_DATA 보스 행만 있음) 수정 없음.

### 영향받는 산출 문서

- `02_design/interface_specification.md` — §2 `GetSkillCost`, §4 `GetCombatantDisplayName`, §8 `RebuildSkillList`/`RebuildTargetList`/`RebuildCoreList`/`TurnQueueUI::RefreshQueue`/`BattleHUD::RefreshStatusText`, §9 `GetActiveBattleManager`.
- `03_test.md` — TC-15~TC-18 추가(NFR-08 스킬·대상·코어 라벨, FR-16 턴 순서·HUD 이름), TC-09의 `(Locked)` → `(잠김)`.
- 클래스 다이어그램: `URPGSkillComponent`, `ARPGBattleManager`, `ARPGPlayerController`에 위 public 메서드 3개 추가. 시퀀스 다이어그램: `RebuildTargetList` → `GetActiveBattleManager` → `GetCombatantDisplayName` 메시지 추가.

---

## 2026-11-15 (3차) — 동료 턴 코어 교체 버그

### 버그 수정

- **동료 턴에 코어 교체 가능 (FR-10)** — `CommandMenu.cpp`, `RPGPlayerController.cpp`
  - 기존: 동료(PC_002, PC_DATA `EquippedCore = None`) 차례에도 커맨드 메뉴에 코어 목록이 나오고 누를 수 있었음. 누르면 `RequestChangeCore` → `ChangeCore`가 동료에게 코어를 장착해 `SkillPool`이 코어 스킬로 바뀜.
  - 결정: 코어 교체는 기본 코어가 있는 캐릭터(현재 PC_001)만 가능.
  - `UCommandMenu::RebuildCoreList`: 행동 캐릭터의 `EquippedCoreID`가 `None`이면 코어 목록을 만들지 않는다(비워 둠).
  - `ARPGPlayerController::RequestChangeCore`: 행동 캐릭터의 `EquippedCoreID`가 `None`이면 경고 로그만 남기고 반환. `ChangeCore`를 호출하지 않고 `bHasChangedCoreThisTurn`도 소모하지 않는다.
  - 기존 함수·변수 이름은 바꾸지 않음.

### 영향받는 산출 문서

- `02_design/interface_specification.md` — §8 `RebuildCoreList` 행 추가, §9 `ARPGPlayerController::RequestChangeCore` 추가.
- `03_test.md` — TC-05a(동료 차례에 코어 목록 숨김, FR-10) 추가.

---

## 2026-11-15 (2차) — FINISH_TASKS T5~T8, StableSort

### 기능 추가 / 버그 수정

- **T5 비용 부족 스킬 선택 불가 (FR-07)** — `RPGSkillcomponent.h/.cpp`, `CommandMenu.cpp`
  - `URPGSkillComponent::CanAffordSkill(FName) const` 추가. SP: `CurrentSP >= cost`, HP: `CurrentHP > cost`(같으면 부족), None/그 외: 충분, 몬스터: 항상 충분.
  - `ExecuteSkill`의 비용 검사가 이 함수를 쓰도록 통일(이전엔 HP를 `<`로 검사해 HP == 비용일 때 실행 허용). 비용 차감은 그대로.
  - `UCommandMenu::RebuildSkillList` 버튼 활성 조건을 `해금 && CanAffordSkill`로 변경. 라벨은 그대로, `SetIsEnabled(false)`만.
- **T6 저장·불러오기 (FR-15)** — `RPGDataManager.h/.cpp`, `RPGBattleManager.h/.cpp`, `PlayerCharacter.cpp`
  - `URPGDataManager::Initialize` override 추가, `Super::Initialize` 뒤 `LoadProgress()` 호출. (1차 changelog의 "LoadProgress 호출 연결 보류"는 과제2 FR-15로 필수가 되어 이번에 반영.)
  - `HandleVictory`에서 **매 전투 승리마다** 플레이어 폰의 `EquippedCoreID`로 `SaveProgress(코어, 0)` 호출 — 코어를 바꾼 뒤 종료해도 복원되게. `ClearedStage`는 기존 Max 보정으로 유지.
  - `HandleStageCleared`의 저장 코어를 `PlayerParty[0]` → 플레이어 폰 기준으로 (`GetPlayerPawnEquippedCoreID` 헬퍼 신설). `PlayerParty` 순서가 보장되지 않아 코어 없는 동료가 저장될 수 있었음.
  - `APlayerCharacter::InitializeCharacterStat`: 세이브의 `EquippedCoreID`가 있고 해금돼 있으면 PC_DATA 기본 코어 대신 장착.
- **T7 튜토리얼 안내 텍스트 (NFR-08)** — `BattleHUD.cpp`
  - 표시 판정을 `InitializeForBattle`(게임 시작 시 1회, 전투 전이라 항상 false) → `HandleBattleStateChanged`로 이동. 전투 중 상태 && `IsTutorialBattle()`이면 Visible, 그 외 Collapsed.
  - `HandleBattleStateChanged`가 `BattleResultText` 없을 때 early return하던 구조를 두 텍스트 독립 처리로 변경.
- **T8 보스전 트리거 (FR-18)** — `FieldEncounter.h`, `RPGBattleManager.h/.cpp`
  - `AFieldEncounter`에 `bIsBossEncounter`, `BossStageNumber`(EditCondition) 추가.
  - `BeginFieldEncounter`에서 BattleManager의 `bIsBossBattle`/`BossStageNumber`를 인카운터 값으로 설정(일반 인카운터면 false/0으로 되돌림).
  - `bAutoStartWithPlacedMonsters`(기본 false) 추가. `BeginPlay`의 "배치된 몬스터로 즉시 전투 시작"은 이 값이 true일 때만.
  - 레벨에는 BattleManager 1개만 두고 보스도 인카운터(`Stage1`/`Boss`)로 시작.
- **T9 일부 — 동률 SPD 순서 고정** — `RPGBattleManager.cpp`
  - `BuildTurnQueue`의 `TurnQueue.Sort` → `StableSort`. SPD가 같으면 수집 순서(아군 먼저)가 유지된다.

### 하지 않은 것

- T9의 `ChangeCore` bool 반환(과제3 U3 의사코드 변경 필요)과 배율 상수 분리(새 클래스 필요) — 이번 학기 보류.

### 영향받는 산출 문서

- `02_design/interface_specification.md` — §2 SkillComponent, §4 BattleManager, §5 FieldEncounter, §6 DataManager `Initialize`, §7 PlayerCharacter, §8 CommandMenu/BattleHUD 갱신.
- `02_design/data_specification.md` — CostType/CostValue 규칙, GD_DATA PC_002 레벨 당김, ENC_DATA 보스·튜토리얼 행, BaseCrit 확정 문구, 엑셀 반영 행 전체.
- `03_test.md` — 신규 작성(FINISH_TASKS §5 TC 11건).

---

## 2026-11-15 (1차) — 버그 4건 수정, 튜토리얼 클리어 저장/해금

### 버그 수정

- **데미지 STR/MAG 분기 버그** (`RPGDamageCalculator.cpp`)
  - 기존: `ElementType == "Physical"`일 때만 STR 사용 → CSV에 `"Physical"` 값이 없어 항상 MAG로 계산됨.
  - 수정: `Slash`/`Strike` → STR, 그 외 → MAG.
- **크리티컬 확률 단위 불일치** (`RPGDamageCalculator.cpp`)
  - 기존: CSV의 `BaseCrit`(비율, 0.05=5%)을 0~100 롤과 그대로 비교 → 실질 확률이 1/100로 줄어듦.
  - 수정: 비교 직전 `BaseCrit * 100`으로 변환.
- **동료 턴 진행 버그** (`RPGBattleManager::StartCurrentActorTurn`)
  - 기존: `Player->GetController()`로 조회 → 레벨에 배치된 동료(PC_002)는 컨트롤러가 없어 턴이 스킵됨.
  - 수정: `UGameplayStatics::GetPlayerController(GetWorld(), 0)` 하나로 파티원 누구의 턴이든 명령.
- **코어 약점/내성 미적용** (`RPGStatComponent::ApplyCoreBonus`)
  - 기존: 코어의 `Weak`/`Resist`가 스탯에 반영되지 않음.
  - 수정: 코어 장착 시 `Weak`/`Resist`를 코어 값으로 덮어씀. 코어 값이 비어있거나 `"None"`이면 `BaseStat` 값 유지.
- **진행도 저장 시 튜토리얼 클리어 플래그 소실 위험** (`RPGDataManager::SaveProgress`)
  - 기존: 매번 새 `URPGSaveGame`을 만들며 `bTutorialCleared`를 채우지 않음.
  - 수정: 기존 `CurrentSaveGame->bTutorialCleared` 값을 이어받도록 수정.

### 기능 추가

- **튜토리얼 클리어 판정/저장/해금**
  - `URPGSaveGame::bTutorialCleared` 필드 추가.
  - `URPGDataManager::SaveTutorialCleared()` 추가 — 기존 `EquippedCoreID`/`ClearedStage`를 이어받아 저장.
  - `ARPGBattleManager::IsTutorialBattle()` 추가 — `PendingSourceEncounter->StageID == "Tutorial"`로 판별.
  - `HandleVictory()`가 `ReturnPartyToField()` 이전에 `IsTutorialBattle()`을 확인해 `SaveTutorialCleared()`를 호출.
  - `IsQuestConditionMet`이 `"TUTORIAL_CLEAR"`를 인식하도록 확장(`IsSkillUnlocked`/`IsCoreUnlocked`에 `bTutorialCleared` 전달).
- **전투 UI 조작 안내 텍스트**
  - `UBattleHUD::TutorialText`(`BindWidgetOptional`), `TutorialMessage`(`EditAnywhere FText`) 추가. (표시 판정 위치는 2차에서 수정.)
