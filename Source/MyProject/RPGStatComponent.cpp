#include "RPGStatComponent.h"

URPGStatComponent::URPGStatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentHP = 0;
}

void URPGStatComponent::InitializeStat(const FMonsterDataRow& InData)
{
    BaseStat = InData;
    MonsterStat = InData;
    CurrentHP = MonsterStat.MaxHP;
    CurrentSP = MonsterStat.MaxSP;
}

void URPGStatComponent::ApplyCoreBonus(const FCoreDataRow& CoreData)
{
    ActiveCoreBonus = CoreData;

    // Always recompute from BaseStat so switching cores never stacks bonuses.
    MonsterStat = BaseStat;
    MonsterStat.MaxHP += CoreData.BonusHP;
    MonsterStat.MaxSP += CoreData.BonusSP;
    MonsterStat.STR += CoreData.BonusSTR;
    MonsterStat.MAG += CoreData.BonusMAG;
    MonsterStat.END += CoreData.BonusEND;
    MonsterStat.SPD += CoreData.BonusSPD;
    MonsterStat.LUK += CoreData.BonusLUK;

    CurrentHP = FMath::Min(CurrentHP, MonsterStat.MaxHP);
    CurrentSP = FMath::Min(CurrentSP, MonsterStat.MaxSP);
}

void URPGStatComponent::ApplyGrowth(const FGrowthDataRow& GrowthData)
{
    BaseStat.MaxHP = GrowthData.MaxHP;
    BaseStat.MaxSP = GrowthData.MaxSP;
    BaseStat.STR = GrowthData.STR;
    BaseStat.MAG = GrowthData.MAG;
    BaseStat.END = GrowthData.END;
    BaseStat.SPD = GrowthData.SPD;
    BaseStat.LUK = GrowthData.LUK;

    const int32 OldMaxHP = MonsterStat.MaxHP;
    const int32 OldMaxSP = MonsterStat.MaxSP;

    // Re-apply whatever Core bonus (if any) was already active on top of the new base stats.
    ApplyCoreBonus(ActiveCoreBonus);

    // Grant the stat increase from leveling immediately, on top of current HP/SP,
    // rather than fully healing (matches the newly-earned max, not a free full heal).
    CurrentHP += FMath::Max(0, MonsterStat.MaxHP - OldMaxHP);
    CurrentSP += FMath::Max(0, MonsterStat.MaxSP - OldMaxSP);
}