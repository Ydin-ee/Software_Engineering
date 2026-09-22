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
