#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RPGDataTypes.h"
#include "RPGStatComponent.generated.h"

UCLASS(ClassGroup = (RPG), meta = (BlueprintSpawnableComponent))
class URPGStatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URPGStatComponent();

    // ���� �� �������� ������ ���̴� ���� ü��
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RPG|Stat")
    int32 CurrentHP;

    // Skill-cost resource (players only; monsters cast for free and never spend this)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RPG|Stat")
    int32 CurrentSP = 0;

    // Player leveling (see FGrowthDataRow / FExpDataRow); monsters stay at level 1.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Growth")
    int32 CurrentLevel = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Growth")
    int32 TotalExp = 0;

    // �������� �ҷ��� ���� ���� ������ (Core ���ʽ� ���� ���� ��ġ)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Stat")
    FMonsterDataRow MonsterStat;

    // Core �ʹ��ʽ��� ���Ե��� ���� ���� ��ġ; ChangeCore�� ���⼭ �ٽ� ����Ѵ�
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Stat")
    FMonsterDataRow BaseStat;

    // The Core bonus currently applied to MonsterStat, kept so ApplyGrowth can re-apply it
    // on top of a freshly leveled-up BaseStat without the caller having to refetch it.
    UPROPERTY()
    FCoreDataRow ActiveCoreBonus;

    // �ܺ�(ĳ����)���� �ҷ��� �����͸� ������ �ʱ�ȭ
    UFUNCTION(BlueprintCallable, Category = "RPG|Init")
    void InitializeStat(const FMonsterDataRow& InData);

    // BaseStat + CoreData ���ʽ��� MonsterStat�� ���. �ھ� ��ü�� ȣ���ص� ���������� ������.
    UFUNCTION(BlueprintCallable, Category = "RPG|Stat")
    void ApplyCoreBonus(const FCoreDataRow& CoreData);

    // Replaces BaseStat with a new level's absolute stat totals (FGrowthDataRow), then
    // re-applies ActiveCoreBonus on top and grants the extra max HP/SP immediately.
    UFUNCTION(BlueprintCallable, Category = "RPG|Growth")
    void ApplyGrowth(const FGrowthDataRow& GrowthData);
};