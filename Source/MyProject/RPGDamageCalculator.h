#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RPGDataTypes.h"
#include "RPGDamageCalculator.generated.h"

// Stateless combat math: damage resolution and evasion checks.
// Pulls attacker/defender stats from their RPGStatComponent and the skill
// definition from URPGDataManager, so no gameplay numbers are hardcoded here.
UCLASS()
class UDamageCalculator : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Resolves final damage for SkillData cast by Attacker onto Defender,
    // applying attack/defense stats, Weak/Resist matchups, and crit chance.
    // Caller looks up SkillData from the correct table (SK_DATA vs SK_MONSTER) first.
    UFUNCTION(BlueprintCallable, Category = "RPG|Combat")
    static int32 CalculateFinalDamage(AActor* Attacker, AActor* Defender, const FSkillDataRow& SkillData);

    // Returns true if the attack is evaded, based on SPD/LUK deltas.
    UFUNCTION(BlueprintCallable, Category = "RPG|Combat")
    static bool CheckEvasion(int32 AtkSPD, int32 DefSPD, int32 AtkLUK, int32 DefLUK);

    // Checks whether ElementType appears in a comma-separated stat field (Weak / Resist).
    // Shared by damage resolution and AI skill-selection heuristics.
    UFUNCTION(BlueprintCallable, Category = "RPG|Combat")
    static bool IsElementListed(const FString& CommaList, FName ElementType);
};
