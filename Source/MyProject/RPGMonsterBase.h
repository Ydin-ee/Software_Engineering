#pragma once

#include "CoreMinimal.h"
#include "RPGCharacterBase.h"
#include "RPGDataTypes.h"
#include "RPGMonsterBase.generated.h"

UCLASS()
class ARPGMonsterBase : public ARPGCharacterBase
{
    GENERATED_BODY()

public:
    ARPGMonsterBase();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Monster")
    FName MonsterID;

    UFUNCTION(BlueprintCallable, Category = "RPG|Init")
    void InitializeMonsterStat();

    // Tracks how threatening each attacker has been (summed damage dealt to this monster),
    // used by AMonsterAIController::SelectTargetBaseOnAggro to pick a target.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Combat")
    TMap<AActor*, int32> AggroTable;

    UFUNCTION(BlueprintCallable, Category = "RPG|Combat")
    void AddAggro(AActor* Source, int32 Amount);
};