#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleArena.generated.h"

// Reusable hidden arena the battle manager teleports combatants into. Place ONE of
// these far from the exploration map (e.g. deep underground) - every field encounter
// teleports into these same slots, so no per-encounter arena setup is needed.
UCLASS()
class ABattleArena : public AActor
{
    GENERATED_BODY()

public:
    ABattleArena();

    // Relative to this actor's transform. Combatants beyond the list length wrap around.
    UPROPERTY(EditAnywhere, Category = "RPG|Arena")
    TArray<FVector> PlayerSlotOffsets = { FVector(0, -150, 0), FVector(0, -50, 0), FVector(0, 50, 0), FVector(0, 150, 0) };

    UPROPERTY(EditAnywhere, Category = "RPG|Arena")
    TArray<FVector> EnemySlotOffsets = { FVector(500, -150, 0), FVector(500, -50, 0), FVector(500, 50, 0), FVector(500, 150, 0) };

    UFUNCTION(BlueprintCallable, Category = "RPG|Arena")
    FTransform GetPlayerSlotTransform(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "RPG|Arena")
    FTransform GetEnemySlotTransform(int32 Index) const;
};
