#include "MonsterAIController.h"
#include "RPGBattleManager.h"
#include "RPGMonsterBase.h"
#include "RPGCharacterBase.h"
#include "RPGStatComponent.h"
#include "RPGSkillComponent.h"
#include "RPGDataManager.h"
#include "RPGDamageCalculator.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void AMonsterAIController::TakeBattleTurn(ARPGBattleManager* BattleManager)
{
    ARPGMonsterBase* MonsterSelf = Cast<ARPGMonsterBase>(GetPawn());
    if (!MonsterSelf || !BattleManager)
    {
        return;
    }

    const TArray<AActor*> PossibleTargets = BattleManager->GetLivingPlayerTargets();
    AActor* Target = SelectTargetBaseOnAggro(MonsterSelf, PossibleTargets);
    const FName SkillID = DetermineOptimalSkill(MonsterSelf, Target);

    // SubmitAction validates turn ownership, executes the skill (if valid),
    // and advances the turn queue either way.
    BattleManager->SubmitAction(MonsterSelf, SkillID, Target);
}

FName AMonsterAIController::DetermineOptimalSkill(ARPGMonsterBase* MonsterSelf, AActor* Target) const
{
    if (!MonsterSelf || !MonsterSelf->SkillComponent || MonsterSelf->SkillComponent->AvailableSkills.Num() == 0)
    {
        return NAME_None;
    }

    UGameInstance* GI = MonsterSelf->GetWorld() ? MonsterSelf->GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;
    if (!DataManager)
    {
        return MonsterSelf->SkillComponent->AvailableSkills[0];
    }

    ARPGCharacterBase* TargetCharacter = Cast<ARPGCharacterBase>(Target);
    const FString TargetWeak = (TargetCharacter && TargetCharacter->StatComponent)
        ? TargetCharacter->StatComponent->MonsterStat.Weak
        : FString();

    FName BestSkill = NAME_None;
    int32 BestScore = -1;

    for (const FName& SkillID : MonsterSelf->SkillComponent->AvailableSkills)
    {
        FSkillDataRow SkillData;
        if (!DataManager->GetMonsterSkillData(SkillID, SkillData))
        {
            continue;
        }

        int32 Score = SkillData.Power;
        if (UDamageCalculator::IsElementListed(TargetWeak, SkillData.ElementType))
        {
            Score += 50; // Prioritize exploiting the target's elemental weakness
        }

        if (Score > BestScore)
        {
            BestScore = Score;
            BestSkill = SkillID;
        }
    }

    return BestSkill.IsNone() ? MonsterSelf->SkillComponent->AvailableSkills[0] : BestSkill;
}

AActor* AMonsterAIController::SelectTargetBaseOnAggro(ARPGMonsterBase* MonsterSelf, const TArray<AActor*>& PossibleTargets) const
{
    if (!MonsterSelf || PossibleTargets.Num() == 0)
    {
        return nullptr;
    }

    AActor* BestTarget = nullptr;
    int32 BestAggro = -1;

    for (AActor* Candidate : PossibleTargets)
    {
        if (!Candidate)
        {
            continue;
        }

        const int32* Aggro = MonsterSelf->AggroTable.Find(Candidate);
        const int32 AggroValue = Aggro ? *Aggro : 0;

        if (AggroValue > BestAggro)
        {
            BestAggro = AggroValue;
            BestTarget = Candidate;
        }
    }

    // Nobody has generated aggro yet (e.g. the first turn) - just pick someone.
    return BestTarget ? BestTarget : PossibleTargets[0];
}
