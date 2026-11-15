# 테스트 케이스

- 프로젝트: Unreal Engine 5 기반 턴제 RPG 전투 시스템
- 작성자: 조의현
- 최초 작성일: 2026-11-15
- 요구사항 기준: 과제2 요구 분석 명세서 (FR-01~19, NFR-01~10)

"실제 결과"와 "Pass/Fail"은 플레이테스트하며 채운다. 로그는 Output Log의 `LogTemp`에서 `[Skill]`, `[Damage]`, `[Save]`, `[BattleManager]`, `[Player]` 태그로 확인한다.

**테스트 전 준비**
- 수정한 CSV를 모두 Reimport했는지 확인 (CD/SK/PC/GD/ENC).
- 기존 세이브 `Saved/SaveGames/RPGSaveSlot.sav`를 삭제한다(세이브 구조가 바뀌었음).
- 레벨: BattleManager 1개(`bAutoStartWithPlacedMonsters = false`), 튜토리얼 인카운터 1개, Stage1 인카운터 3개, 보스 인카운터 1개(`bIsBossEncounter`, `BossStageNumber = 1`), PC_002 배치.

---

## 1. 범위 내 테스트 케이스

| TC | 요구사항 | 사전 조건 | 절차 | 기대 결과 | 실제 결과 | Pass/Fail |
|---|---|---|---|---|---|---|
| TC-01 데미지 계산 (물리) | FR-08 | PC_001 Lv1 + COR_001(STR 20, LUK 10), 대상 MOB_001(END 3) | SK_001(Slash, Power 15)로 공격. 크리티컬이 뜨지 않은 경우의 `[Skill] … FinalDamage` 로그 확인 | FinalDamage = **24** (15 + 20×0.5 − 3×0.5 = 23.5 → 24) | | |
| TC-02 데미지 계산 (마법·약점) | FR-08 | PC_001 Lv1 + COR_002(MAG 20), 대상 MOB_001(Weak Fire) | SK_005(Fire, Power 35)로 공격, 비크리티컬 로그 확인 | FinalDamage = **65** (43.5 × 1.5 = 65.25 → 65) | | |
| TC-03 크리티컬 확률 | FR-08 | TC-01과 동일 | SK_001로 30회 이상 공격, `[Damage] … critical hit!` 로그 횟수 집계 | 치명률 6% 근처 (30회 중 0~4회 수준). 크리티컬 시 데미지 36(24 × 1.5) | | |
| TC-04 코어 교체 약점/내성 | FR-10 | PC_001, COR_002 해금 상태 | ① COR_001 장착 상태에서 PC_001 `MonsterStat.Weak/Resist` 확인 ② COR_002로 교체 ③ 다음 턴 COR_001로 복귀 | ① Weak `Elec`, Resist 공란 ② Weak `Grass`, Resist `Fire` ③ Weak `Elec`, Resist 공란(BaseStat 값으로 복귀) | | |
| TC-05 코어 교체 제한 | FR-10 | PC_001 턴, COR_002 해금 | ① 코어 교체 ② 같은 턴에 다시 코어 목록 확인 ③ 같은 턴에 스킬 사용 | ① 성공 ② 코어 목록이 비어 있음(두 번째 교체 불가) ③ 스킬 사용 가능, 턴 소모 | | |
| TC-05a 동료 차례 코어 목록 숨김 | FR-10 | PC_001 + PC_002 파티, COR_002 해금 | ① PC_002 차례에 커맨드 메뉴의 코어 목록 확인 ② (블루프린트 등으로 `RequestChangeCore(COR_002)`를 강제 호출) ③ 다음 PC_001 차례에 코어 목록 확인 | ① 코어 목록이 비어 있음(COR_001/COR_002 버튼 없음), 스킬 목록은 PC_002의 SkillPool 그대로 ② `[PlayerController] … has no core - core change ignored.` 로그, PC_002 스킬 목록 변화 없음 ③ PC_001 코어 목록 정상 표시, 교체 가능(교체 횟수 미소모) | | |
| TC-06 동료 턴 진행 | FR-06 | PC_001 + PC_002 2인 파티로 전투 진입 | 턴 순서가 PC_002에 도달할 때까지 진행 | PC_002 차례에 커맨드 메뉴가 뜨고, PC_002의 SkillPool(SK_002/007/008/011/012)이 목록에 보인다. 턴이 스킵되지 않는다 | | |
| TC-07 비용 부족 스킬 비활성 (SP) | FR-07 | PC_001 + COR_002, SP < 10 | 커맨드 메뉴의 SK_005(SP 10) 버튼 상태 확인 | SK_005 버튼이 비활성(클릭 불가), 라벨은 그대로. SP ≥ 10이면 활성 | | |
| TC-08 비용 부족 스킬 비활성 (HP) | FR-07 | PC_001 + COR_001, SK_003(HP 12.5%) 해금 | HP를 비용(RoundToInt(MaxHP × 0.125))과 같거나 적게 만든 뒤 메뉴 확인 | SK_003 버튼 비활성. HP가 비용보다 1 이상 크면 활성 | | |
| TC-09 튜토리얼 해금 | FR-11, FR-18 | 새 게임(세이브 없음) | ① 튜토리얼 전투 전 코어 목록 확인 ② 튜토리얼 인카운터(`Tutorial`/`Battle1`) 승리 ③ 다음 전투에서 코어 목록 확인 | ① COR_002 비활성 "(Locked)" ② `[Save] Tutorial cleared.` 로그 ③ COR_002 활성 | | |
| TC-10 재실행 복원 | FR-15 | TC-09 완료 후 COR_002로 교체하고 전투 1회 승리 | ① 게임 종료 ② 재실행, `[Save] Progress loaded` 로그 확인 ③ 시작 시 장착 코어·코어 목록 확인 ④ (보스 클리어 후) 재실행 | ② Core: COR_002, Tutorial: cleared ③ COR_002 장착 상태로 시작, COR_002 활성 ④ ClearedStage 1 유지 | | |
| TC-11 튜토리얼 플래그 보존 | FR-15 | 튜토리얼 클리어 상태 | 보스 인카운터 승리 후 재실행 | `[Save] Progress loaded` 로그에 Tutorial: cleared, ClearedStage: 1 — 보스전 저장이 튜토리얼 클리어를 false로 되돌리지 않음 | | |
| TC-12 안내 텍스트 | NFR-08 | `WBP_BattleHUD`에 `TutorialText` 추가, `TutorialMessage` 입력 | ① 필드(전투 전) ② 튜토리얼 전투 중 ③ 튜토리얼 승리 후(BattleEnd) ④ Stage1 전투 중 | ① 안 보임 ② 보임 ③ 안 보임 ④ 안 보임 | | |
| TC-12a 안내 텍스트 위젯 없음 | NFR-08 | `WBP_BattleHUD`에서 `TutorialText`를 임시로 제거 | 빌드·실행 후 튜토리얼 전투 진입 | 빌드 성공, 실행 중 크래시·에러 로그 없음 | | |
| TC-13 보스전 트리거 | FR-18 | 레벨에 보스 인카운터(`Stage1`/`Boss`, `bIsBossEncounter`, `BossStageNumber = 1`) | ① 게임 시작 직후 상태 확인 ② Stage1 일반 인카운터 승리 후 `[Save]` 로그 ③ 보스 인카운터 접촉 ④ 승리 후 `[Save]` 로그 | ① 전투가 자동 시작되지 않음(필드 상태) ② `ClearedStage: 0` ③ MOB_003과 전투 ④ `ClearedStage: 1` | | |
| TC-14 전체 플레이 | NFR-06 | 새 게임 | 튜토리얼 → Stage1 Battle1~3 → 보스 클리어를 **10회** 반복 | 10회 모두 멈춤·크래시 없이 클리어까지 진행 | 회차별 기록: 1 / 2 / 3 / 4 / 5 / 6 / 7 / 8 / 9 / 10 | |

## 2. 범위 밖 (이번 학기 미테스트)

- 스테이지 2·3 인카운터, PC_003 합류, COR_003·004 해금
- 보스전 승리 시 동료 합류(`SpawnCompanion`) — 확장 기능. PC_002가 이미 파티에 있어 이번 범위에서는 동작하지 않는 것이 정상
- 상태이상(`Status`/`StatusChance`), 아이템, 회복 구역, 방어/도주, 보스 강기술 조건, 게임오버 UI

## 3. 비고

- 위 TC는 FINISH_TASKS.md §5의 필수 항목이다. 기존 구현분(필드↔전투 전환, SPD 턴 순서, 몬스터 AI 타겟/스킬 선택, EXP/레벨업)에 대한 TC는 과제2의 해당 FR ID를 확인한 뒤 추가한다.
