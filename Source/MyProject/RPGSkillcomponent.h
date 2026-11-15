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

    // Single source of truth for the resource-cost check, shared by ExecuteSkill and
    // UCommandMenu::RebuildSkillList so the menu never offers a skill that would then fail.
    // SP: CurrentSP >= cost. HP: CurrentHP > cost (equal counts as unaffordable - the
    // caster would drop). None/other cost types are always affordable. Monsters cast for free.
    UFUNCTION(BlueprintCallable, Category = "RPG|Skill")
    bool CanAffordSkill(FName SkillID) const;

    UPROPERTY(BlueprintAssignable, Category = "RPG|Skill")
    FOnSkillResolved OnSkillResolved;
};