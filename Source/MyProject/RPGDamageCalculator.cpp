#include "RPGDamageCalculator.h"
#include "RPGCharacterBase.h"
#include "RPGStatComponent.h"

bool UDamageCalculator::IsElementListed(const FString& CommaList, FName ElementType)
{
    if (CommaList.IsEmpty() || ElementType.IsNone())
    {
        return false;
    }

    TArray<FString> Elements;
    CommaList.ParseIntoArray(Elements, TEXT(","), true);

    const FString ElementString = ElementType.ToString();
    for (const FString& Element : Elements)
    {
        if (Element.TrimStartAndEnd().Equals(ElementString, ESearchCase::IgnoreCase))
        {
            return true;
        }
    }
    return false;
}

int32 UDamageCalculator::CalculateFinalDamage(AActor* Attacker, AActor* Defender, const FSkillDataRow& SkillData)
{
    ARPGCharacterBase* AttackerCharacter = Cast<ARPGCharacterBase>(Attacker);
    ARPGCharacterBase* DefenderCharacter = Cast<ARPGCharacterBase>(Defender);
    if (!AttackerCharacter || !DefenderCharacter || !AttackerCharacter->StatComponent || !DefenderCharacter->StatComponent)
    {
        return 0;
    }

    const FMonsterDataRow& AtkStat = AttackerCharacter->StatComponent->MonsterStat;
    const FMonsterDataRow& DefStat = DefenderCharacter->StatComponent->MonsterStat;

    // Physical skills (Slash/Strike) scale off STR, elemental/magic skills scale off MAG.
    const bool bIsPhysical = SkillData.ElementType == FName(TEXT("Slash")) || SkillData.ElementType == FName(TEXT("Strike"));
    const int32 AttackPower = bIsPhysical ? AtkStat.STR : AtkStat.MAG;

    float RawDamage = static_cast<float>(SkillData.Power) + (AttackPower * 0.5f) - (DefStat.END * 0.5f);
    RawDamage = FMath::Max(1.0f, RawDamage);

    // One log line per applied modifier so playtests (TC-01~03) can tell which path a hit took.
    if (IsElementListed(DefStat.Weak, SkillData.ElementType))
    {
        RawDamage *= 1.5f;
        UE_LOG(LogTemp, Warning, TEXT("[Damage] WEAK: %s hit %s with %s (%s) x1.5"),
            *AttackerCharacter->GetName(), *DefenderCharacter->GetName(), *SkillData.SkillName.ToString(), *SkillData.ElementType.ToString());
    }
    else if (IsElementListed(DefStat.Resist, SkillData.ElementType))
    {
        RawDamage *= 0.5f;
        UE_LOG(LogTemp, Warning, TEXT("[Damage] RESIST: %s hit %s with %s (%s) x0.5"),
            *AttackerCharacter->GetName(), *DefenderCharacter->GetName(), *SkillData.SkillName.ToString(), *SkillData.ElementType.ToString());
    }

    // BaseCrit from CSV is a ratio (0.05 = 5%); convert to percentage points to compare against the 0~100 roll.
    const float CritChance = (SkillData.BaseCrit * 100.f) + (AtkStat.LUK * 0.1f);
    if (FMath::FRandRange(0.f, 100.f) < CritChance)
    {
        RawDamage *= 1.5f;
        UE_LOG(LogTemp, Warning, TEXT("[Damage] CRITICAL: %s hit %s with %s x1.5 (chance %.1f%%)"),
            *AttackerCharacter->GetName(), *DefenderCharacter->GetName(), *SkillData.SkillName.ToString(), CritChance);
    }

    return FMath::Max(1, FMath::RoundToInt(RawDamage));
}

bool UDamageCalculator::CheckEvasion(int32 AtkSPD, int32 DefSPD, int32 AtkLUK, int32 DefLUK)
{
    const float EvasionChance = FMath::Clamp(5.0f + (DefSPD - AtkSPD) * 0.5f + (DefLUK - AtkLUK) * 0.3f, 0.0f, 75.0f);
    return FMath::FRandRange(0.f, 100.f) < EvasionChance;
}
