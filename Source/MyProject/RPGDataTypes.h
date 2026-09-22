#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RPGDataTypes.generated.h"

// ���� �����͸� ���� ����ü
USTRUCT(BlueprintType)
struct FMonsterDataRow : public FTableRowBase
{
    GENERATED_BODY()


public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    FName MonsterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    int32 MaxHP = 0;

    // Only meaningful for players (monsters cast for free); no MD_DATA column, defaults to 0.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    int32 MaxSP = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    int32 STR = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    int32 MAG = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    int32 END = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    int32 SPD = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    int32 LUK = 0;

    // ���� �� ���� (�ϴ� ���ڿ��� �޾ƿ���, ���� Enum�̳� GameplayTag�� �Ľ�)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    FString Weak;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    FString Resist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    int32 DropEXP = 0;

    // ���� ���� ��ų 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Monster")
    FString MonsterSkills;
};

// ��ų �� �ھ� �����͸� ���� ����ü
USTRUCT(BlueprintType)
struct FSkillDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") FName SkillName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") FName ElementType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") FName CostType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") float CostValue = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") int32 Power = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") float BaseCrit = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") FName TargetType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") FName Status;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") float StatusChance = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") FString Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Skill") FName Quest;   // ��ų �ر� ���� (��: STAGE1_CLEAR, None)
};

// Player Core (equippable loadout) data: flat stat bonuses + granted skills
USTRUCT(BlueprintType)
struct FCoreDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") FName CoreName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") FName CoreElement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") int32 BonusHP = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") int32 BonusSP = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") int32 BonusSTR = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") int32 BonusMAG = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") int32 BonusEND = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") int32 BonusSPD = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") int32 BonusLUK = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") FString Weak;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") FString Resist;

    // Comma-separated skill IDs (SK_DATA rows) granted while this core is equipped
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") FString CoreSkills;

    // Core unlock condition (e.g. "STAGE1_CLEAR", "None"), same format as FSkillDataRow::Quest
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Core") FName Quest;
};

// Player base stats (per-character, before any Core bonus). Row name = CharacterID (e.g. PC_001).
USTRUCT(BlueprintType)
struct FCharacterDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") FName Name;

    // Element of this character's unarmed/basic attack (Slash, Strike, ...)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") FName BaseAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") int32 MaxHP = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") int32 MaxSP = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") int32 STR = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") int32 MAG = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") int32 END = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") int32 SPD = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") int32 LUK = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") FString Weak;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") FString Resist;

    // "None" for Core-driven characters (e.g. the protagonist); otherwise the core auto-equipped on init.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") FName EquippedCore;

    // Comma-separated innate skills for characters that don't use a Core (e.g. companions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") FString SkillPool;

    // Party-join condition (e.g. "STAGE1_CLEAR", "None")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Character") FName Quest;
};

// Per-level absolute stat totals for one character. Not looked up by row name (the sheet
// repeats CharacterID per level, which can't be a unique DataTable row key) - RPGDataManager
// scans every row and matches CharacterID + Level instead.
USTRUCT(BlueprintType)
struct FGrowthDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") FName CharacterID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") int32 Level = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") int32 MaxHP = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") int32 MaxSP = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") int32 STR = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") int32 MAG = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") int32 END = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") int32 SPD = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Growth") int32 LUK = 0;
};

// EXP required to leave a given level. Looked up by scanning for matching Level (see FGrowthDataRow note).
// NOTE: the sheet's headers are "Exp To Next Level" / "Total Exp" (with spaces) - DataTable CSV
// import matches column headers to these exact property names, so rename the sheet's header
// row to "ExpToNextLevel" / "TotalExp" (no spaces) before importing.
USTRUCT(BlueprintType)
struct FExpDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Exp") int32 Level = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Exp") int32 ExpToNextLevel = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Exp") int32 TotalExp = 0;
};

// One scripted field encounter: which mobs spawn for a given Stage+Battle. MobList/NumberList
// are "|"-delimited parallel lists (e.g. "MOB_001|MOB_002" and "2|2"). Looked up by scanning
// for matching Stage + Battle (the sheet repeats Stage per battle, so it isn't a unique row key).
USTRUCT(BlueprintType)
struct FEncounterRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Encounter") FName Stage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Encounter") FName Battle;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Encounter") FString Mob;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data|Encounter") FString Number;
};