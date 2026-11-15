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
| `ElementType` | FName | 데미지 공격력 산출 기준. **`Slash` / `Strike` → `STR`로 스케일링, 그 외 모든 값(예: `Fire`, `Ice` 등 원소 속성) → `MAG`로 스케일링.** CSV에 `"Physical"` 값은 쓰지 않는다(기존에 이 값이 전제돼 있었으나 실제로는 존재하지 않아 전부 MAG로 계산되는 버그가 있었음, 2026-11-15 수정). |
| `BaseCrit` | float | **CSV에는 비율로 저장한다 (0.05 = 5%).** 코드(`UDamageCalculator::CalculateFinalDamage`)에서 `BaseCrit * 100`으로 변환한 뒤 `AtkStat.LUK * 0.1`을 더해 0~100 크리티컬 롤과 비교한다. |
| `StatusChance` | float | `BaseCrit`과 동일한 단위 규칙(비율)을 쓸 예정. **이번 학기 범위에서는 상태이상 기능 자체가 범위 밖이라 코드에서 아직 참조하지 않는다.** 상태이상 기능을 구현할 때 `* 100` 변환을 넣어야 한다. |
| `Quest` | FName | 스킬 해금 조건. `"None"`(항시 해금) / `"STAGE<N>_CLEAR"`(해당 스테이지 클리어 시) / `"TUTORIAL_CLEAR"`(튜토리얼 클리어 시, 2026-11-15 추가). 인식 못 하는 문자열은 fail-closed(false)로 처리. |

## 2. FCoreDataRow (CD_DATA)

| 컬럼 | 타입 | 단위/규칙 |
|---|---|---|
| `Weak` / `Resist` | FString (쉼표 구분) | **코어 장착 시 캐릭터의 `Weak`/`Resist`를 이 값으로 덮어쓴다** (`URPGStatComponent::ApplyCoreBonus`, 2026-11-15 추가). 값이 비어 있거나 `"None"`이면 캐릭터 기본값(`BaseStat.Weak`/`BaseStat.Resist`, 즉 `PC_DATA`의 값)을 그대로 유지한다. `PC_001`처럼 `PC_DATA`에 `Weak`/`Resist`를 공란으로 둔 캐릭터는 코어가 전적으로 약점/내성을 결정하게 된다. |
| `Quest` | FName | 코어 해금 조건. `FSkillDataRow::Quest`와 동일한 포맷(`"None"` / `"STAGE<N>_CLEAR"` / `"TUTORIAL_CLEAR"`). `COR_002`는 `"Tutorial"` → **`"TUTORIAL_CLEAR"`로 수정 필요** (엑셀 반영은 사용자 작업, 아래 §4 참고). |

## 3. FMonsterDataRow / FCharacterDataRow (MD_DATA / PC_DATA)

| 컬럼 | 타입 | 단위/규칙 |
|---|---|---|
| `Weak` / `Resist` | FString (쉼표 구분) | 캐릭터/몬스터의 기본 약점·내성. 플레이어 캐릭터(`PC_DATA`)는 코어가 약점/내성을 결정하는 설계 의도상 공란으로 둘 수 있다(§2 참고). |

## 4. URPGSaveGame (세이브 데이터, DataTable 아님)

| 필드 | 타입 | 규칙 |
|---|---|---|
| `EquippedCoreID` | FName | 마지막으로 장착한 코어. |
| `ClearedStage` | int32 | 클리어한 최고 스테이지. `SaveProgress` 호출 시 기존 값보다 낮게 내려가지 않는다. |
| `bTutorialCleared` | bool | **(신규, 2026-11-15)** 튜토리얼 전투(필드 인카운터 `StageID == "Tutorial"`) 승리 시 true. `SaveProgress`/`SaveTutorialCleared` 양쪽 모두 기존 `CurrentSaveGame`의 값을 이어받아 서로 덮어쓰지 않는다. |

---

## 엑셀/CSV 반영 필요 항목 (사용자 작업)

아래는 Claude Code가 코드만 맞춰둔 상태이고, `GameData.xlsx` → CSV 반영 및 DataTable Reimport는 사용자가 진행해야 한다 (CLAUDE.md §4).

```csv
# CD_DATA.csv 에서 COR_002 행
Quest: Tutorial → TUTORIAL_CLEAR
```

- `BaseCrit` / `StatusChance` 값이 현재 0~100 스케일로 잘못 입력돼 있다면(예: `5`로 입력된 경우) **비율(0.05)로 수정**해야 한다. 기존 값이 이미 0~1 비율이었다면 변경할 필요 없음 — 엑셀 원본 값을 확인해 알려주면 같이 점검하겠다.
