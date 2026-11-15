# 변경 이력

- 프로젝트: Unreal Engine 5 기반 턴제 RPG 전투 시스템
- 작성자: 조의현

날짜는 커밋 날짜 기준으로 기록한다.

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
