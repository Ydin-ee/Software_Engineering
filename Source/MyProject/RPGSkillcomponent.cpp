#include "RPGSkillComponent.h"
#include "RPGDataManager.h"
#include "RPGDamageCalculator.h"
#include "RPGCharacterBase.h"
#include "RPGMonsterBase.h"
#include "RPGStatComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

URPGSkillComponent::URPGSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool URPGSkillComponent::ExecuteSkill(AActor* Target, FName SkillID)
{
    if (!AvailableSkills.Contains(SkillID))
    {
        UE_LOG(LogTemp, Error, TEXT("[Skill] %s not in AvailableSkills."), *SkillID.ToString());
        return false;
    }

    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;
    if (!DataManager)
    {
        return false;
    }

    // Monsters and players are imported from separate sheets (SK_MONSTER vs SK_DATA)
    // so the two ID spaces cannot collide with each other.
    const bool bIsMonsterOwner = Cast<ARPGMonsterBase>(GetOwner()) != nullptr;

    FSkillDataRow SkillData;
    const bool bFoundSkill = bIsMonsterOwner
        ? DataManager->GetMonsterSkillData(SkillID, SkillData)
        : DataManager->GetSkillData(SkillID, SkillData);

    if (!bFoundSkill)
    {
        UE_LOG(LogTemp, Error, TEXT("[Skill] Data not found for %s."), *SkillID.ToString());
        return false;
    }

    // Player skills can be quest-gated (FSkillDataRow::Quest, e.g. "STAGE3_CLEAR")
    // against the save file's ClearedStage; monster skills are not gated this way.
    if (!bIsMonsterOwner && !DataManager->IsSkillUnlocked(SkillID))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Skill] %s is locked (requires %s)."),
            *SkillID.ToString(), *SkillData.Quest.ToString());
        return false;
    }

    FString TargetName = Target ? Target->GetName() : TEXT("None");

    ARPGCharacterBase* AttackerCharacter = Cast<ARPGCharacterBase>(GetOwner());
    ARPGCharacterBase* DefenderCharacter = Cast<ARPGCharacterBase>(Target);
    if (!AttackerCharacter || !DefenderCharacter || !AttackerCharacter->StatComponent || !DefenderCharacter->StatComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("[Skill] Invalid attacker/defender for %s."), *SkillData.SkillName.ToString());
        return false;
    }

    // Resource cost - monsters cast for free. Charged even on a miss (paid on attempt,
    // not on hit), and blocks the skill entirely if the cost can't be afforded.
    if (!bIsMonsterOwner)
    {
        URPGStatComponent* AttackerStat = AttackerCharacter->StatComponent;

        if (SkillData.CostType == FName(TEXT("SP")))
        {
            const int32 SPCost = FMath::RoundToInt(SkillData.CostValue);
            if (AttackerStat->CurrentSP < SPCost)
            {
                UE_LOG(LogTemp, Warning, TEXT("[Skill] %s cannot afford SP cost for %s (%d/%d SP)."),
                    *AttackerCharacter->GetName(), *SkillData.SkillName.ToString(), AttackerStat->CurrentSP, SPCost);
                return false;
            }
            AttackerStat->CurrentSP -= SPCost;
        }
        else if (SkillData.CostType == FName(TEXT("HP")))
        {
            const int32 HPCost = FMath::RoundToInt(SkillData.CostValue * AttackerStat->MonsterStat.MaxHP);
            if (AttackerStat->CurrentHP < HPCost)
            {
                UE_LOG(LogTemp, Warning, TEXT("[Skill] %s cannot afford HP cost for %s (%d/%d HP)."),
                    *AttackerCharacter->GetName(), *SkillData.SkillName.ToString(), AttackerStat->CurrentHP, HPCost);
                return false;
            }
            AttackerCharacter->TakeRPGDamage(HPCost, FName(TEXT("SelfCost")));
        }
    }

    const FMonsterDataRow& AtkStat = AttackerCharacter->StatComponent->MonsterStat;
    const FMonsterDataRow& DefStat = DefenderCharacter->StatComponent->MonsterStat;

    if (UDamageCalculator::CheckEvasion(AtkStat.SPD, DefStat.SPD, AtkStat.LUK, DefStat.LUK))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Skill] %s used %s on %s -> MISSED!"),
            *AttackerCharacter->GetName(), *SkillData.SkillName.ToString(), *TargetName);
        OnSkillResolved.Broadcast(Target, 0, true);
        return true;
    }

    const int32 FinalDamage = UDamageCalculator::CalculateFinalDamage(AttackerCharacter, DefenderCharacter, SkillData);

    UE_LOG(LogTemp, Warning, TEXT("[Skill] %s used %s on %s (Power: %d, FinalDamage: %d)"),
        *AttackerCharacter->GetName(), *SkillData.SkillName.ToString(), *TargetName, SkillData.Power, FinalDamage);

    DefenderCharacter->TakeRPGDamage(FinalDamage, SkillData.ElementType);

    // Feed the AI's threat table so AMonsterAIController::SelectTargetBaseOnAggro
    // can react to who has been hitting it hardest.
    if (ARPGMonsterBase* DefenderMonster = Cast<ARPGMonsterBase>(DefenderCharacter))
    {
        DefenderMonster->AddAggro(AttackerCharacter, FinalDamage);
    }

    OnSkillResolved.Broadcast(Target, FinalDamage, false);

    return true;
}
