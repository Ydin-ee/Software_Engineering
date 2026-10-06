# Turn-based RPG Battle System

영남대학교 소프트웨어공학 개인 프로젝트
작성자: 조의현 (EUIHYUN JO)

## 1. 프로젝트 개요

- **주제**: Unreal Engine 5 기반 턴제 RPG 전투 시스템 (프로토타입)
- **개발 엔진/언어**: Unreal Engine 5.8, C++
- **한 줄 설명**: 필드에서 몬스터와 접촉하면 레벨 전환 없이 전투 아레나로 이동해 턴제 전투를 진행하고, 전투 중 "코어"를 교체해 스탯과 스킬을 바꿔가며 싸우는 RPG 프로토타입
- **개발 범위**: 튜토리얼 → 스테이지1(잡몹 전투 3회) → 보스전 → 클리어 (상세 범위는 프로젝트 관리 문서 참고)

## 2. 개발 환경

- Unreal Engine 버전: 5.8
- IDE: Visual Studio 2022
- 사용 언어: C++
- 실행 방법: `MyProject.uproject` 실행 → Editor에서 Play

## 4. 프로젝트 문서 (SW 개발 수명주기)

단계별 문서는 `/docs` 폴더에 정리하며, 코드 변경 시 관련 문서를 같은 커밋에서 함께 갱신한다.

| 단계 | 문서 | 내용 | 상태 |
|---|---|---|---|
| 프로젝트 관리 | [docs/00_project_management.md](docs/00_project_management.md) | 범위 정의, 규모 산정, WBS, 일정·위험 관리 | 완료 |
| 요구사항 분석 | docs/01_requirements.md | 기능/비기능 요구사항 | 범위 축소 반영 후 업로드 예정 |
| 분석 및 설계 | docs/02_design.md | 클래스·시퀀스 다이어그램, 데이터·인터페이스 명세 | 범위 축소 반영 후 업로드 예정 |
| 구현 | 소스코드 (Source/, Content/) | - | 진행 중 |
| 테스트 | docs/03_test.md | 테스트 케이스 및 결과 | 예정 |
| 유지보수/변경 이력 | docs/04_changelog.md | 버전별 수정 내역 | 예정 |
