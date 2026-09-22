#include "PlayerCharacter.h"
#include "RPGStatComponent.h"
#include "RPGSkillComponent.h"
#include "RPGDataManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

APlayerCharacter::APlayerCharacter()
{
    bHasReceivedShift = false;
    EquippedCoreID = NAME_None;
    CharacterID = TEXT("PC_001"); // Default test ID
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    InitializeCharacterStat();
}

void APlayerCharacter::InitializeCharacterStat()
{
    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;
    if (!DataManager || !StatComponent)
    {
        return;
    }

    FCharacterDataRow CharData;
    if (!DataManager->GetCharacterData(CharacterID, CharData))
    {
        UE_LOG(LogTemp, Error, TEXT("[Player] Character data not found for %s."), *CharacterID.ToString());
        return;
    }

    FMonsterDataRow BaseStatRow;
    BaseStatRow.MonsterName = CharData.Name;
    BaseStatRow.MaxHP = CharData.MaxHP;
    BaseStatRow.MaxSP = CharData.MaxSP;
    BaseStatRow.STR = CharData.STR;
    BaseStatRow.MAG = CharData.MAG;
    BaseStatRow.END = CharData.END;
    BaseStatRow.SPD = CharData.SPD;
    BaseStatRow.LUK = CharData.LUK;
    BaseStatRow.Weak = CharData.Weak;
    BaseStatRow.Resist = CharData.Resist;

    StatComponent->InitializeStat(BaseStatRow);

    if (!CharData.EquippedCore.IsNone() && !CharData.EquippedCore.ToString().Equals(TEXT("None"), ESearchCase::IgnoreCase))
    {
        // Core-driven character (e.g. the protagonist): the Core grants the skill list.
        ChangeCore(CharData.EquippedCore);
    }
    else if (SkillComponent)
    {
        // Core-less character (e.g. a companion): skills come straight from SkillPool.
        TArray<FString> SkillIDStrings;
        CharData.SkillPool.ParseIntoArray(SkillIDStrings, TEXT(","), true);

        SkillComponent->AvailableSkills.Empty();
        for (const FString& SkillIDString : SkillIDStrings)
        {
            SkillComponent->AvailableSkills.Add(FName(*SkillIDString.TrimStartAndEnd()));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[Player] %s initialized as %s. HP:%d/%d SP:%d/%d"),
        *CharacterID.ToString(), *CharData.Name.ToString(),
        StatComponent->CurrentHP, StatComponent->MonsterStat.MaxHP,
        StatComponent->CurrentSP, StatComponent->MonsterStat.MaxSP);
}

void APlayerCharacter::GainExp(int32 Amount)
{
    if (!StatComponent || Amount <= 0)
    {
        return;
    }

    StatComponent->TotalExp += Amount;
    UE_LOG(LogTemp, Warning, TEXT("[EXP] %s gained %d EXP (Total: %d)"), *GetName(), Amount, StatComponent->TotalExp);

    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;
    if (!DataManager)
    {
        return;
    }

    // ExpDataRow::TotalExp is the cumulative EXP needed to advance PAST that level;
    // ExpToNextLevel == 0 marks the level cap, so stop there even if TotalExp is met.
    FExpDataRow ExpRow;
    while (DataManager->GetExpRequirement(StatComponent->CurrentLevel, ExpRow)
        && ExpRow.ExpToNextLevel > 0
        && StatComponent->TotalExp >= ExpRow.TotalExp)
    {
        StatComponent->CurrentLevel++;

        FGrowthDataRow GrowthRow;
        if (DataManager->GetGrowthData(CharacterID, StatComponent->CurrentLevel, GrowthRow))
        {
            StatComponent->ApplyGrowth(GrowthRow);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Level Up] No GrowthData row for %s at level %d."),
                *CharacterID.ToString(), StatComponent->CurrentLevel);
        }

        UE_LOG(LogTemp, Warning, TEXT("[Level Up] %s reached level %d! HP:%d/%d SP:%d/%d"),
            *GetName(), StatComponent->CurrentLevel,
            StatComponent->CurrentHP, StatComponent->MonsterStat.MaxHP,
            StatComponent->CurrentSP, StatComponent->MonsterStat.MaxSP);
    }
}

void APlayerCharacter::ChangeCore(FName CoreID)
{
    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;
    if (!DataManager)
    {
        return;
    }

    if (!DataManager->IsCoreUnlocked(CoreID))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] Core %s is locked."), *CoreID.ToString());
        return;
    }

    FCoreDataRow CoreData;
    if (!DataManager->GetCoreData(CoreID, CoreData))
    {
        UE_LOG(LogTemp, Error, TEXT("[Player] Core data not found for %s."), *CoreID.ToString());
        return;
    }

    const bool bFirstEquip = EquippedCoreID.IsNone();
    EquippedCoreID = CoreID;

    if (StatComponent)
    {
        StatComponent->ApplyCoreBonus(CoreData);

        // First-ever core equip starts the player at full HP/SP for the new max.
        if (bFirstEquip)
        {
            StatComponent->CurrentHP = StatComponent->MonsterStat.MaxHP;
            StatComponent->CurrentSP = StatComponent->MonsterStat.MaxSP;
        }
    }

    if (SkillComponent)
    {
        // Skills granted by a core replace those from the previous core.
        TArray<FString> SkillIDStrings;
        CoreData.CoreSkills.ParseIntoArray(SkillIDStrings, TEXT(","), true);

        SkillComponent->AvailableSkills.Empty();
        for (const FString& SkillIDString : SkillIDStrings)
        {
            SkillComponent->AvailableSkills.Add(FName(*SkillIDString.TrimStartAndEnd()));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[Player] Core changed to %s (%s) - STR+%d MAG+%d END+%d SPD+%d LUK+%d, HP:%d/%d"),
        *CoreID.ToString(), *CoreData.CoreName.ToString(),
        CoreData.BonusSTR, CoreData.BonusMAG, CoreData.BonusEND, CoreData.BonusSPD, CoreData.BonusLUK,
        StatComponent ? StatComponent->CurrentHP : 0,
        StatComponent ? StatComponent->MonsterStat.MaxHP : 0);
}
