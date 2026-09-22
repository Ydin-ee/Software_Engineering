#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

class ARPGBattleManager;
class ARPGMonsterBase;

UCLASS()
class AMonsterAIController : public AAIController
{
    GENERATED_BODY()

public:
    // Called by ARPGBattleManager when it becomes this monster's turn.
    // Picks a target and a skill, submits the action, and lets the
    // battle manager advance the turn queue.
    UFUNCTION(BlueprintCallable, Category = "RPG|AI")
    void TakeBattleTurn(ARPGBattleManager* BattleManager);

    // Scores MonsterSelf's AvailableSkills against Target's Weak/Resist and picks the
    // highest-scoring one (data-driven: reads FSkillDataRow via URPGDataManager).
    UFUNCTION(BlueprintCallable, Category = "RPG|AI")
    FName DetermineOptimalSkill(ARPGMonsterBase* MonsterSelf, AActor* Target) const;

    // Picks whichever living candidate has drawn the most aggro from MonsterSelf;
    // falls back to the first candidate if nothing has generated aggro yet.
    UFUNCTION(BlueprintCallable, Category = "RPG|AI")
    AActor* SelectTargetBaseOnAggro(ARPGMonsterBase* MonsterSelf, const TArray<AActor*>& PossibleTargets) const;
};
