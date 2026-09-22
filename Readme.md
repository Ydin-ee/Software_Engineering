# [프로젝트명] - Turn-based RPG Battle System

영남대학교 소프트웨어공학 개인 프로젝트
작성자: 조의현 (EUIHYUN JO)

## 1. 프로젝트 개요

- **주제**: (자유주제 - 예: 턴제 RPG 배틀 시스템)
- **개발 엔진/언어**: Unreal Engine 5, C++
- **한 줄 설명**: (예: 몬스터와의 턴제 전투, 스킬 시스템, 필드 인카운터를 갖춘 RPG 배틀 프로토타입)

## 2. 개발 환경

- Unreal Engine 버전: 5.x
- IDE: Visual Studio 2022
- 사용 언어: C++
- 실행 방법: `MyProject.uproject` 실행 → Editor에서 Play

## 3. 기능 요약

- 턴제 배틀 시스템 (RPGBattleManager)
- 몬스터 AI (MonsterAIController)
- 스킬 시스템 (RPGSkillComponent)
- 필드 인카운터 (FieldEncounter)
- 세이브/로드 (RPGSaveGame)
- 전투 UI / 데미지 플로팅 텍스트

## 4. 프로젝트 문서 (SW 개발 수명주기)

과제 요구사항에 맞춰 단계별 문서를 `/docs` 폴더에 정리 예정. 문서 수정 시 관련 문서도 함께 갱신 예정

| 단계 | 문서 | 비고 |
|---|---|---|
| 요구사항 분석 | [docs/01_requirements.md](docs/01_requirements.md) | 기능/비기능 요구사항 |
| 분석 및 설계 | [docs/02_design.md](docs/02_design.md) | 클래스 다이어그램, 시스템 구조 |
| 구현 | 소스코드 (Source/, Content/) | - |
| 테스트 | [docs/03_test.md](docs/03_test.md) | 테스트 케이스 및 결과 |
| 유지보수/변경 이력 | [docs/04_changelog.md](docs/04_changelog.md) | 버전별 수정 내역 |

## 5. 폴더 구조
MyProject/
├── Source/ # C++ 소스 코드
├── Content/ # 게임 에셋
├── Config/ # 엔진/게임 설정
├── docs/ # 단계별 산출 문서
└── Readme.md
