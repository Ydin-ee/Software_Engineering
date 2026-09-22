#include "RPGDataManager.h"
#include "RPGSaveGame.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const FString RPGSaveSlotName = TEXT("RPGSaveSlot");
    const int32 RPGSaveUserIndex = 0;

    // Quest format is "STAGE<N>_CLEAR" or "None" (see FSkillDataRow::Quest).
    bool IsQuestConditionMet(FName Quest, int32 ClearedStage)
    {
        if (Quest.IsNone())
        {
            return true;
        }

        FString QuestString = Quest.ToString();
        if (QuestString.Equals(TEXT("None"), ESearchCase::IgnoreCase))
        {
            return true;
        }

        if (QuestString.StartsWith(TEXT("STAGE"), ESearchCase::IgnoreCase))
        {
            QuestString.RightChopInline(5); // strip "STAGE"
            QuestString.RemoveFromEnd(TEXT("_CLEAR"), ESearchCase::IgnoreCase);
            const int32 RequiredStage = FCString::Atoi(*QuestString);
            return ClearedStage >= RequiredStage;
        }

        // Unrecognized quest tag: fail closed rather than silently unlocking it.
        return false;
    }
}

URPGDataManager::URPGDataManager()
{
    static ConstructorHelpers::FObjectFinder<UDataTable> DT_Monster(TEXT("/Game/3DRPGPROJECT/csv/MD_DATA.MD_DATA"));
    if (DT_Monster.Succeeded())
    {
        MonsterDataTable = DT_Monster.Object;
    }

    static ConstructorHelpers::FObjectFinder<UDataTable> DT_Skill(TEXT("/Game/3DRPGPROJECT/csv/SK_DATA.SK_DATA"));
    if (DT_Skill.Succeeded())
    {
        SkillDataTable = DT_Skill.Object;
    }

    static ConstructorHelpers::FObjectFinder<UDataTable> DT_MonsterSkill(TEXT("/Game/3DRPGPROJECT/csv/SK_MONSTER.SK_MONSTER"));
    if (DT_MonsterSkill.Succeeded())
    {
        MonsterSkillDataTable = DT_MonsterSkill.Object;
    }

    static ConstructorHelpers::FObjectFinder<UDataTable> DT_Core(TEXT("/Game/3DRPGPROJECT/csv/CD_DATA.CD_DATA"));
    if (DT_Core.Succeeded())
    {
        CoreDataTable = DT_Core.Object;
    }

    static ConstructorHelpers::FObjectFinder<UDataTable> DT_Character(TEXT("/Game/3DRPGPROJECT/csv/PC_DATA.PC_DATA"));
    if (DT_Character.Succeeded())
    {
        CharacterDataTable = DT_Character.Object;
    }

    static ConstructorHelpers::FObjectFinder<UDataTable> DT_Growth(TEXT("/Game/3DRPGPROJECT/csv/GD_DATA.GD_DATA"));
    if (DT_Growth.Succeeded())
    {
        GrowthDataTable = DT_Growth.Object;
    }

    static ConstructorHelpers::FObjectFinder<UDataTable> DT_Exp(TEXT("/Game/3DRPGPROJECT/csv/EXP_DATA.EXP_DATA"));
    if (DT_Exp.Succeeded())
    {
        ExpDataTable = DT_Exp.Object;
    }

    static ConstructorHelpers::FObjectFinder<UDataTable> DT_Encounter(TEXT("/Game/3DRPGPROJECT/csv/ENC_DATA.ENC_DATA"));
    if (DT_Encounter.Succeeded())
    {
        EncounterDataTable = DT_Encounter.Object;
    }
}

bool URPGDataManager::GetMonsterData(FName RowName, FMonsterDataRow& OutRow)
{
    if (MonsterDataTable)
    {
        FString ContextString = TEXT("Monster Data Search Context");
        FMonsterDataRow* FoundRow = MonsterDataTable->FindRow<FMonsterDataRow>(RowName, ContextString);
        if (FoundRow)
        {
            OutRow = *FoundRow;
            return true;
        }
    }
    return false;
}

bool URPGDataManager::GetSkillData(FName SkillID, FSkillDataRow& OutRow)
{
    if (SkillDataTable)
    {
        static const FString Ctx(TEXT("Skill Data Search Context"));
        if (FSkillDataRow* Found = SkillDataTable->FindRow<FSkillDataRow>(SkillID, Ctx))
        {
            OutRow = *Found;
            return true;
        }
    }
    return false;
}

bool URPGDataManager::GetMonsterSkillData(FName SkillID, FSkillDataRow& OutRow)
{
    if (MonsterSkillDataTable)
    {
        static const FString Ctx(TEXT("Monster Skill Data Search Context"));
        if (FSkillDataRow* Found = MonsterSkillDataTable->FindRow<FSkillDataRow>(SkillID, Ctx))
        {
            OutRow = *Found;
            return true;
        }
    }
    return false;
}

void URPGDataManager::SaveProgress(FName EquippedCoreID, int32 ClearedStage)
{
    URPGSaveGame* NewSave = Cast<URPGSaveGame>(UGameplayStatics::CreateSaveGameObject(URPGSaveGame::StaticClass()));
    if (!NewSave)
    {
        return;
    }

    // Never let a stale/lower call regress progress already recorded.
    const int32 PreviousClearedStage = CurrentSaveGame ? CurrentSaveGame->ClearedStage : 0;
    ClearedStage = FMath::Max(ClearedStage, PreviousClearedStage);

    NewSave->EquippedCoreID = EquippedCoreID;
    NewSave->ClearedStage = ClearedStage;

    if (UGameplayStatics::SaveGameToSlot(NewSave, RPGSaveSlotName, RPGSaveUserIndex))
    {
        CurrentSaveGame = NewSave;
        UE_LOG(LogTemp, Warning, TEXT("[Save] Progress saved. Core: %s, ClearedStage: %d"),
            *EquippedCoreID.ToString(), ClearedStage);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Save] Failed to save progress."));
    }
}

bool URPGDataManager::LoadProgress()
{
    if (!UGameplayStatics::DoesSaveGameExist(RPGSaveSlotName, RPGSaveUserIndex))
    {
        return false;
    }

    CurrentSaveGame = Cast<URPGSaveGame>(UGameplayStatics::LoadGameFromSlot(RPGSaveSlotName, RPGSaveUserIndex));
    return CurrentSaveGame != nullptr;
}

bool URPGDataManager::GetCoreData(FName CoreID, FCoreDataRow& OutRow)
{
    if (CoreDataTable)
    {
        static const FString Ctx(TEXT("Core Data Search Context"));
        if (FCoreDataRow* Found = CoreDataTable->FindRow<FCoreDataRow>(CoreID, Ctx))
        {
            OutRow = *Found;
            return true;
        }
    }
    return false;
}

bool URPGDataManager::IsSkillUnlocked(FName SkillID) const
{
    if (!SkillDataTable)
    {
        return false;
    }

    static const FString Ctx(TEXT("Skill Unlock Check"));
    FSkillDataRow* Found = SkillDataTable->FindRow<FSkillDataRow>(SkillID, Ctx);
    if (!Found)
    {
        return false;
    }

    const int32 ClearedStage = CurrentSaveGame ? CurrentSaveGame->ClearedStage : 0;
    return IsQuestConditionMet(Found->Quest, ClearedStage);
}

void URPGDataManager::GetAllCoreIDs(TArray<FName>& OutCoreIDs) const
{
    OutCoreIDs.Reset();
    if (CoreDataTable)
    {
        OutCoreIDs = CoreDataTable->GetRowNames();
    }
}

bool URPGDataManager::IsCoreUnlocked(FName CoreID) const
{
    if (!CoreDataTable)
    {
        return false;
    }

    static const FString Ctx(TEXT("Core Unlock Check"));
    FCoreDataRow* Found = CoreDataTable->FindRow<FCoreDataRow>(CoreID, Ctx);
    if (!Found)
    {
        return false;
    }

    const int32 ClearedStage = CurrentSaveGame ? CurrentSaveGame->ClearedStage : 0;
    return IsQuestConditionMet(Found->Quest, ClearedStage);
}

bool URPGDataManager::GetCharacterData(FName CharacterID, FCharacterDataRow& OutRow)
{
    if (CharacterDataTable)
    {
        static const FString Ctx(TEXT("Character Data Search Context"));
        if (FCharacterDataRow* Found = CharacterDataTable->FindRow<FCharacterDataRow>(CharacterID, Ctx))
        {
            OutRow = *Found;
            return true;
        }
    }
    return false;
}

bool URPGDataManager::GetGrowthData(FName CharacterID, int32 Level, FGrowthDataRow& OutRow)
{
    if (!GrowthDataTable)
    {
        return false;
    }

    // RowID (GD_001, ...) is just a unique key for the CSV/DataTable; the actual lookup
    // is by CharacterID + Level, so every row is scanned rather than using FindRow.
    bool bFound = false;
    GrowthDataTable->ForeachRow<FGrowthDataRow>(TEXT("Growth Data Search Context"),
        [&bFound, &OutRow, CharacterID, Level](const FName& RowID, const FGrowthDataRow& Row)
        {
            if (!bFound && Row.CharacterID == CharacterID && Row.Level == Level)
            {
                OutRow = Row;
                bFound = true;
            }
        });
    return bFound;
}

bool URPGDataManager::GetExpRequirement(int32 Level, FExpDataRow& OutRow)
{
    if (!ExpDataTable)
    {
        return false;
    }

    TArray<FExpDataRow*> Rows;
    ExpDataTable->GetAllRows<FExpDataRow>(TEXT("Exp Data Search Context"), Rows);
    for (const FExpDataRow* Row : Rows)
    {
        if (Row->Level == Level)
        {
            OutRow = *Row;
            return true;
        }
    }
    return false;
}

bool URPGDataManager::GetEncounterData(FName Stage, FName Battle, FEncounterRow& OutRow)
{
    if (!EncounterDataTable)
    {
        return false;
    }

    // RowID (ENC_001, ...) is just a unique key for the CSV/DataTable; the actual lookup
    // is by Stage + Battle, so every row is scanned rather than using FindRow.
    bool bFound = false;
    EncounterDataTable->ForeachRow<FEncounterRow>(TEXT("Encounter Data Search Context"),
        [&bFound, &OutRow, Stage, Battle](const FName& RowID, const FEncounterRow& Row)
        {
            if (!bFound && Row.Stage == Stage && Row.Battle == Battle)
            {
                OutRow = Row;
                bFound = true;
            }
        });
    return bFound;
}

void URPGDataManager::ParseEncounterMobList(const FEncounterRow& EncounterRow, TArray<FName>& OutMobIDs, TArray<int32>& OutCounts)
{
    OutMobIDs.Reset();
    OutCounts.Reset();

    // "|"-delimited parallel lists, e.g. Mob="MOB_001|MOB_002", Number="2|2".
    TArray<FString> MobStrings;
    TArray<FString> CountStrings;
    EncounterRow.Mob.ParseIntoArray(MobStrings, TEXT("|"), true);
    EncounterRow.Number.ParseIntoArray(CountStrings, TEXT("|"), true);

    for (int32 Index = 0; Index < MobStrings.Num(); ++Index)
    {
        OutMobIDs.Add(FName(*MobStrings[Index].TrimStartAndEnd()));
        const int32 Count = CountStrings.IsValidIndex(Index) ? FCString::Atoi(*CountStrings[Index]) : 1;
        OutCounts.Add(Count);
    }
}