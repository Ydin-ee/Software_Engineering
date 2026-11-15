#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RPGSaveGame.generated.h"

UCLASS()
class URPGSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    URPGSaveGame();

    // Which Core (skill loadout / equipment) the player currently has equipped.
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RPG|Save")
    FName EquippedCoreID;

    // Highest stage the player has cleared; drives FSkillDataRow::Quest gating.
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RPG|Save")
    int32 ClearedStage;

    // Set once the tutorial battle (FieldEncounter StageID == "Tutorial") is won;
    // drives the "TUTORIAL_CLEAR" quest gate (e.g. COR_002's unlock).
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RPG|Save")
    bool bTutorialCleared;
};
