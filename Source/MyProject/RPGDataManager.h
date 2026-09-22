#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RPGDataTypes.h"
#include "RPGDataManager.generated.h"

class URPGSaveGame;

UCLASS()
class URPGDataManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    URPGDataManager(); // ������ ����
    // �����Ϳ��� ��� ���� ������ ���̺� ������ ������ ������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Data")
    class UDataTable* MonsterDataTable;

    // ������ ��ȯ ��� ����(&)�� ����Ͽ� ��������Ʈ���� �����ϰ� �����͸� ���� �� �ְ� ����
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool GetMonsterData(FName RowName, FMonsterDataRow& OutRow);

    UPROPERTY() UDataTable* SkillDataTable;
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool GetSkillData(FName SkillID, FSkillDataRow& OutRow);

    // Separate table for monster-only skills (SK_MONSTER), kept apart from the
    // player skill sheet (SK_DATA) so the two ID spaces cannot collide.
    UPROPERTY() UDataTable* MonsterSkillDataTable;
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool GetMonsterSkillData(FName SkillID, FSkillDataRow& OutRow);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Data")
    UDataTable* CoreDataTable;
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool GetCoreData(FName CoreID, FCoreDataRow& OutRow);

    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    void GetAllCoreIDs(TArray<FName>& OutCoreIDs) const;

    // Gates a core against FCoreDataRow::Quest, same convention as IsSkillUnlocked.
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool IsCoreUnlocked(FName CoreID) const;

    // Row name = CharacterID (e.g. PC_001).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Data")
    UDataTable* CharacterDataTable;
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool GetCharacterData(FName CharacterID, FCharacterDataRow& OutRow);

    // Scanned (CharacterID, Level) pairs aren't unique row names - see FGrowthDataRow.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Data")
    UDataTable* GrowthDataTable;
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool GetGrowthData(FName CharacterID, int32 Level, FGrowthDataRow& OutRow);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Data")
    UDataTable* ExpDataTable;
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool GetExpRequirement(int32 Level, FExpDataRow& OutRow);

    // Scanned (Stage, Battle) pairs aren't unique row names - see FEncounterRow.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Data")
    UDataTable* EncounterDataTable;
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    bool GetEncounterData(FName Stage, FName Battle, FEncounterRow& OutRow);

    // Splits FEncounterRow::Mob / Number ("MOB_001|MOB_002", "2|2") into matched
    // (MobID, Count) pairs at the same index. Not wired into any spawn logic itself -
    // a ready-to-use helper for whoever builds that (see ARPGBattleManager::BeginFieldEncounter).
    UFUNCTION(BlueprintCallable, Category = "RPG|Data")
    static void ParseEncounterMobList(const FEncounterRow& EncounterRow, TArray<FName>& OutMobIDs, TArray<int32>& OutCounts);

    // --- Save / Load ---

    UPROPERTY(BlueprintReadOnly, Category = "RPG|Save")
    URPGSaveGame* CurrentSaveGame;

    UFUNCTION(BlueprintCallable, Category = "RPG|Save")
    void SaveProgress(FName EquippedCoreID, int32 ClearedStage);

    UFUNCTION(BlueprintCallable, Category = "RPG|Save")
    bool LoadProgress();

    // Gates a player skill against FSkillDataRow::Quest (e.g. "STAGE3_CLEAR", "None")
    // using CurrentSaveGame->ClearedStage.
    UFUNCTION(BlueprintCallable, Category = "RPG|Save")
    bool IsSkillUnlocked(FName SkillID) const;
};