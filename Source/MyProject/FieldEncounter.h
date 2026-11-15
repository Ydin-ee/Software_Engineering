#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FieldEncounter.generated.h"

class USphereComponent;
class UStaticMeshComponent;

// A roaming field symbol (grayboxed as a generic slime). Touching it hands off to
// ARPGBattleManager::BeginFieldEncounter, which looks up StageID/BattleID in
// Level_Encounter_Table to decide which monsters actually spawn in the arena.
UCLASS()
class AFieldEncounter : public AActor
{
    GENERATED_BODY()

public:
    AFieldEncounter();

    UPROPERTY(EditAnywhere, Category = "RPG|Encounter")
    FName StageID;

    UPROPERTY(EditAnywhere, Category = "RPG|Encounter")
    FName BattleID;

    // Boss encounters start through the same trigger; BeginFieldEncounter copies these onto
    // the (single) ARPGBattleManager's bIsBossBattle / BossStageNumber for that one battle.
    UPROPERTY(EditAnywhere, Category = "RPG|Encounter")
    bool bIsBossEncounter = false;

    UPROPERTY(EditAnywhere, Category = "RPG|Encounter", meta = (EditCondition = "bIsBossEncounter"))
    int32 BossStageNumber = 0;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    USphereComponent* TriggerVolume;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* SlimeMesh;

    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
