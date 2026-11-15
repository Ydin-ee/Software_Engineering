# 변경 이력

- 프로젝트: Unreal Engine 5 기반 턴제 RPG 전투 시스템
- 작성자: 조의현

날짜는 커밋 날짜 기준으로 기록한다.

---

## 2026-11-15

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
  - 기존: 코어의 `Weak`/`Resist`가 스탯에 반영되지 않음(`BaseStat` 값 그대로 유지됨).
  - 수정: 코어 장착 시 `Weak`/`Resist`를 코어 값으로 덮어씀. 코어 값이 비어있거나 `"None"`이면 `BaseStat` 값 유지.
- **진행도 저장 시 튜토리얼 클리어 플래그 소실 위험** (`RPGDataManager::SaveProgress`)
  - 기존: `SaveProgress`가 매번 새 `URPGSaveGame`을 만들며 `bTutorialCleared`를 채우지 않음 → 보스전 클리어로 재저장하면 튜토리얼 클리어 기록이 false로 덮어써질 수 있었음.
  - 수정: 기존 `CurrentSaveGame->bTutorialCleared` 값을 이어받도록 수정.

### 기능 추가

- **튜토리얼 클리어 판정/저장/해금**
  - `URPGSaveGame::bTutorialCleared` 필드 추가.
  - `URPGDataManager::SaveTutorialCleared()` 추가 — 기존 `EquippedCoreID`/`ClearedStage`를 이어받아 저장.
  - `ARPGBattleManager::IsTutorialBattle()` 추가 — `PendingSourceEncounter->StageID == "Tutorial"`로 판별.
  - `HandleVictory()`가 `ReturnPartyToField()`(= `PendingSourceEncounter` 정리) 이전에 `IsTutorialBattle()`을 확인해 `SaveTutorialCleared()`를 호출.
  - `URPGDataManager`의 `IsQuestConditionMet` 내부 판정 함수가 `"TUTORIAL_CLEAR"` 문자열을 인식하도록 확장 (`IsSkillUnlocked`/`IsCoreUnlocked`에도 `bTutorialCleared` 전달).
- **전투 UI 조작 안내 텍스트**
  - `UBattleHUD::TutorialText`(`BindWidgetOptional`), `TutorialMessage`(`EditAnywhere FText`) 추가.
  - `InitializeForBattle`에서 `IsTutorialBattle()`이면 `TutorialText`를 표시, 아니면 Collapsed.

### 영향받는 산출 문서

- `02_design/interface_specification.md` — 위 변경된/신규 함수 시그니처 반영.
- `02_design/data_specification.md` — `ElementType`/`BaseCrit`/`StatusChance` 단위 규칙, `Weak`/`Resist` 코어 덮어쓰기 규칙, `Quest`의 `"TUTORIAL_CLEAR"` 값, `bTutorialCleared` 필드 반영.

### 비고

- `(선택) 게임 시작 시 LoadProgress() 호출 연결`은 이번 커밋에서 보류함 (todo.md: 한 번에 끝까지 플레이하는 맛보기 버전이면 생략 가능).
