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

    // Physical skills scale off STR, elemental/magic skills scale off MAG.
    const bool bIsPhysical = SkillData.ElementType == FName(TEXT("Physical"));
    const int32 AttackPower = bIsPhysical ? AtkStat.STR : AtkStat.MAG;

    float RawDamage = static_cast<float>(SkillData.Power) + (AttackPower * 0.5f) - (DefStat.END * 0.5f);
    RawDamage = FMath::Max(1.0f, RawDamage);

    if (IsElementListed(DefStat.Weak, SkillData.ElementType))
    {
        RawDamage *= 1.5f;
    }
    else if (IsElementListed(DefStat.Resist, SkillData.ElementType))
    {
        RawDamage *= 0.5f;
    }

    const float CritChance = SkillData.BaseCrit + (AtkStat.LUK * 0.1f);
    if (FMath::FRandRange(0.f, 100.f) < CritChance)
    {
        RawDamage *= 1.5f;
        UE_LOG(LogTemp, Warning, TEXT("[Damage] %s landed a critical hit!"), *AttackerCharacter->GetName());
    }

    return FMath::Max(1, FMath::RoundToInt(RawDamage));
}

bool UDamageCalculator::CheckEvasion(int32 AtkSPD, int32 DefSPD, int32 AtkLUK, int32 DefLUK)
{
    const float EvasionChance = FMath::Clamp(5.0f + (DefSPD - AtkSPD) * 0.5f + (DefLUK - AtkLUK) * 0.3f, 0.0f, 75.0f);
    return FMath::FRandRange(0.f, 100.f) < EvasionChance;
}
