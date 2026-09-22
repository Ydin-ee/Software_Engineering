#pragma once

#include "CoreMinimal.h"
#include "RPGCharacterBase.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class APlayerCharacter : public ARPGCharacterBase
{
    GENERATED_BODY()

public:
    APlayerCharacter();

protected:
    virtual void BeginPlay() override;

public:
    // Row name into CharacterData (e.g. PC_001). Set per-instance/BP for companions.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Player")
    FName CharacterID;

    // Loads FCharacterDataRow for CharacterID into StatComponent, auto-equips its
    // EquippedCore if any, and grants SkillPool for characters that don't use a Core.
    // Called from BeginPlay, but also public so a newly spawned companion can be
    // initialized immediately after SpawnActor (before its own BeginPlay would run).
    UFUNCTION(BlueprintCallable, Category = "RPG|Init")
    void InitializeCharacterStat();

    // Awards EXP and applies any level-ups (FExpDataRow/FGrowthDataRow) that result.
    UFUNCTION(BlueprintCallable, Category = "RPG|Growth")
    void GainExp(int32 Amount);
    // ����Ʈ(����) Ŀ�ǵ带 ���� �� �ִ��� ����
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RPG|Player")
    bool bHasReceivedShift;

    // ���� ���� ���� �ھ� ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Player")
    FName EquippedCoreID;

    // �ھ� ��ü �Լ� (CoreData ������ ���� ����)
    UFUNCTION(BlueprintCallable, Category = "RPG|Player")
    void ChangeCore(FName CoreID);
};