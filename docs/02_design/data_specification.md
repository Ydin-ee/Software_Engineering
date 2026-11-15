# 데이터 명세서

- 프로젝트: Unreal Engine 5 기반 턴제 RPG 전투 시스템
- 작성자: 조의현
- 최초 작성일: 2026-11-15

데이터 흐름: 엑셀(`GameData.xlsx`) → CSV(`docs/data/`) → `/Game/3DRPGPROJECT/csv/` DataTable → `URPGDataManager` → 각 컴포넌트.
데이터 컬럼·값의 의미가 바뀌면 같은 커밋에서 이 문서도 갱신한다.

---

## 1. FSkillDataRow (SK_DATA / SK_MONSTER)

| 컬럼 | 타입 | 단위/규칙 |
|---|---|---|
| `ElementType` | FName | 데미지 공격력 산출 기준. **`Slash` / `Strike` → `STR`로 스케일링, 그 외 모든 값(`Fire`, `Ice`, `Elec`, `Grass`, `Heal` 등) → `MAG`로 스케일링.** CSV에 `"Physical"` 값은 쓰지 않는다. |
| `CostType` / `CostValue` | FName / float | `SP`: `CostValue`는 SP 절대량(예: SK_005 = 10). `HP`: `CostValue`는 MaxHP 대비 비율(예: SK_003 = 0.125 → MaxHP의 12.5%). `None`: 비용 없음. 비용 충분 판정은 `URPGSkillComponent::CanAffordSkill` 참고(HP는 현재 HP가 비용보다 **커야** 함). 몬스터 스킬(SK_MONSTER)은 비용 검사를 하지 않는다. |
| `BaseCrit` | float | **CSV에는 비율로 저장한다 (0.05 = 5%).** 코드(`UDamageCalculator::CalculateFinalDamage`)에서 `치명률(%) = BaseCrit × 100 + LUK × 0.1`로 변환해 0~100 롤과 비교한다. 현재 CSV 값(SK_001 0.05, SK_003 0.15 등)은 이미 비율이라 변경할 필요 없다. |
| `StatusChance` | float | `BaseCrit`과 같은 비율 단위(SK_005 0.15 등, 이미 비율). **상태이상은 이번 학기 범위 밖이라 코드에서 아직 참조하지 않는다.** 구현 시 `× 100` 변환을 넣어야 한다. |
| `Quest` | FName | 스킬 해금 조건. `None`(항시 해금) / `STAGE<N>_CLEAR`(해당 스테이지 클리어 시) / `TUTORIAL_CLEAR`(튜토리얼 클리어 시). 인식 못 하는 문자열은 fail-closed(false). |

## 2. FCoreDataRow (CD_DATA)

| 컬럼 | 타입 | 단위/규칙 |
|---|---|---|
| `Weak` / `Resist` | FString (쉼표 구분) | **코어 장착 시 캐릭터의 `Weak`/`Resist`를 이 값으로 덮어쓴다** (`URPGStatComponent::ApplyCoreBonus`, FR-10). 값이 비어 있거나 `None`이면 캐릭터 기본값(`BaseStat`, 즉 PC_DATA의 값)을 그대로 유지한다. PC_001처럼 PC_DATA의 `Weak`/`Resist`가 공란인 캐릭터는 코어가 전적으로 약점/내성을 결정한다. 예: COR_001 → Weak `Elec`, Resist 공란 / COR_002 → Weak `Grass`, Resist `Fire`. |
| `Quest` | FName | 코어 해금 조건. `FSkillDataRow::Quest`와 동일한 포맷. COR_002는 `TUTORIAL_CLEAR`. |

## 3. FMonsterDataRow / FCharacterDataRow (MD_DATA / PC_DATA)

| 컬럼 | 타입 | 단위/규칙 |
|---|---|---|
| `Weak` / `Resist` | FString (쉼표 구분) | 캐릭터/몬스터의 기본 약점·내성. 플레이어 캐릭터(PC_DATA)는 코어가 약점/내성을 결정하는 설계 의도상 공란으로 둘 수 있다(§2). |
| `EquippedCore` (PC_DATA) | FName | 시작 시 장착하는 기본 코어. `None`이면 코어 없는 캐릭터(동료)로 취급해 `SkillPool`을 그대로 쓴다. 세이브에 장착 코어가 있고 해금돼 있으면 세이브 값이 우선한다(FR-15). |
| `Quest` (PC_DATA) | FName | 캐릭터 해금 조건. PC_002는 처음부터 파티에 있으므로 `None`. |

## 4. FGrowthDataRow (GD_DATA)

| 컬럼 | 규칙 |
|---|---|
| `CharacterID`, `Level` | (CharacterID, Level) 쌍으로 조회. 레벨업 시 해당 레벨의 **절대 스탯**으로 `BaseStat`을 교체한다. |
| PC_002 행 | 기획 원본은 Lv 10~30이었으나, PC_002가 1레벨부터 파티에 있으므로 **Level 값을 9씩 당겨 1~21로 사용**한다(10→1, 11→2, …, 30→21). 다른 열은 그대로. 원본 Lv 10 행의 값이 PC_DATA 기본 스탯과 같아 1레벨 시작과 자연스럽게 이어진다. |

## 5. FEncounterRow (ENC_DATA)

| 컬럼 | 규칙 |
|---|---|
| `Stage`, `Battle` | (Stage, Battle) 쌍으로 조회. `AFieldEncounter`의 `StageID`/`BattleID`와 일치해야 한다. |
| `Mob`, `Number` | `|`로 구분된 몬스터 ID와 마릿수를 같은 인덱스로 짝지음(예: `MOB_001|MOB_002`, `1|2`). |
| 이번 범위 행 | `Tutorial/Battle1 = MOB_001×1`, `Stage1/Battle1 = MOB_001×2`, `Stage1/Battle2 = MOB_002×2`, `Stage1/Battle3 = MOB_001|MOB_002 1|2`, **`Stage1/Boss = MOB_003×1`**(보스도 인카운터로 시작, 레벨에 몬스터 직접 배치 X). `StageID == "Tutorial"`인 인카운터로 시작한 전투가 튜토리얼 전투다. Stage1 Battle4·5, Stage2·3 행은 범위 밖이지만 코드에 영향이 없어 그대로 둔다. |

## 6. URPGSaveGame (세이브 데이터, DataTable 아님)

| 필드 | 타입 | 규칙 |
|---|---|---|
| `EquippedCoreID` | FName | 플레이어 폰(주인공)의 장착 코어. **매 전투 승리 시** 저장되고(FR-15), 시작 시 해금돼 있으면 복원된다. |
| `ClearedStage` | int32 | 클리어한 최고 스테이지. 보스전 승리 시 `BossStageNumber`로 저장되며, 어떤 `SaveProgress` 호출로도 기존 값보다 낮아지지 않는다. |
| `bTutorialCleared` | bool | 튜토리얼 전투 승리 시 true. `SaveProgress`/`SaveTutorialCleared` 양쪽 모두 기존 값을 이어받아 서로 덮어쓰지 않는다. |

세이브 슬롯: `"RPGSaveSlot"` (UserIndex 0). 게임 시작 시 `URPGDataManager::Initialize`에서 자동으로 불러온다. 레벨/EXP는 저장하지 않는다(범위 밖).

---

## 엑셀/CSV 반영 필요 항목 (사용자 작업)

Claude Code는 코드만 맞춰둔 상태이며, `GameData.xlsx` → CSV 반영 및 DataTable Reimport(`/Game/3DRPGPROJECT/csv/`)는 사용자가 진행한다. `docs/data/`의 CSV를 고쳤다면 엑셀에도 같이 반영한다.

```csv
# CD_DATA.csv — COR_002 행의 Quest
COR_002,화염술사의 코어,Fire,0,30,0,15,10,10,8,Grass,Fire,"SK_001,SK_003,SK_005,SK_006",TUTORIAL_CLEAR

# SK_DATA.csv — Quest → None (SK_001, SK_002, SK_005는 이미 None)
SK_003,슬래시,Slash,HP,0.125,35,0.15,Enemy_All,None,0,"적 전체에 참격 속성 소 데미지, 크리티컬 확률 중간",None
SK_006,화염 베기,Fire,SP,28,35,0,Enemy_All,Burn,0.2,적 전체에 화염 속성 소 데미지,None
SK_007,부흐,Ice,SP,10,35,0,Enemy_Single,Freeze,0.15,적 1체에 빙결 속성 소 데미지,None
SK_008,블리자드 에지,Ice,SP,28,60,0,Enemy_Single,Freeze,0.2,적 1체에 빙결 속성 중 데미지,None
SK_011,갈,Grass,SP,10,35,0,Enemy_Single,None,0,적 1체에 질풍 속성 소 데미지,None
SK_012,디아,Heal,SP,10,40,0,Ally_Single,None,0,아군 1체 HP회복 소,None

# PC_DATA.csv — PC_002 행의 Quest
PC_002,동료1,Strike,120,150,5,25,10,10,5,Grass,Fire,None,"SK_002,SK_007,SK_008,SK_011,SK_012",None

# GD_DATA.csv — PC_002 21행의 Level만 9씩 당김 (RowID·다른 열 그대로)
GD_011,PC_002,1,120,150,5,25,10,10,5
GD_013,PC_002,2,123,155,6,29,11,11,6
GD_015,PC_002,3,126,160,7,32,12,12,7
GD_017,PC_002,4,129,165,8,36,13,13,8
GD_019,PC_002,5,132,170,9,40,14,14,9
GD_021,PC_002,6,135,175,10,44,15,15,10
GD_023,PC_002,7,138,180,11,48,17,16,11
GD_025,PC_002,8,141,185,12,52,19,17,12
GD_027,PC_002,9,144,190,13,56,21,18,13
GD_029,PC_002,10,147,195,14,60,23,19,14
GD_031,PC_002,11,150,200,15,64,25,20,15
GD_034,PC_002,12,155,208,17,69,27,21,16
GD_037,PC_002,13,160,216,19,74,29,22,17
GD_040,PC_002,14,165,224,21,79,31,23,18
GD_043,PC_002,15,170,232,22,84,33,24,19
GD_046,PC_002,16,175,240,24,90,35,25,20
GD_049,PC_002,17,180,250,26,100,37,26,21
GD_052,PC_002,18,185,262,28,105,38,27,22
GD_055,PC_002,19,190,274,30,120,40,28,23
GD_058,PC_002,20,195,286,32,125,42,29,24
GD_061,PC_002,21,200,300,34,140,45,30,25

# ENC_DATA.csv — 수정 2행 + 추가 2행 (EncounterID는 비어 있는 번호로)
ENC_002,Stage1,Battle2,MOB_002,2
ENC_003,Stage1,Battle3,MOB_001|MOB_002,1|2
ENC_016,Tutorial,Battle1,MOB_001,1
ENC_017,Stage1,Boss,MOB_003,1
```

`BaseCrit`/`StatusChance`는 이미 비율이라 변경 없음.
