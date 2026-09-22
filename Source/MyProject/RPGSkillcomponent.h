#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RPGDataTypes.h"
#include "RPGSkillComponent.generated.h"

// Broadcast once per ExecuteSkill call so UI (UDamageFloatingText, UBattleHUD) can react
// without polling: bMissed true means DamageAmount is meaningless (evaded).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillResolved, AActor*, Target, int32, DamageAmount, bool, bMissed);

UCLASS(ClassGroup = (RPG), meta = (BlueprintSpawnableComponent))
class URPGSkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URPGSkillComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Skill")
    TArray<FName> AvailableSkills;

    UFUNCTION(BlueprintCallable, Category = "RPG|Skill")
    bool ExecuteSkill(AActor* Target, FName SkillID);

    UPROPERTY(BlueprintAssignable, Category = "RPG|Skill")
    FOnSkillResolved OnSkillResolved;
};