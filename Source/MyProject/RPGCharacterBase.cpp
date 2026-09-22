#include "RPGCharacterBase.h"
#include "RPGStatComponent.h"
#include "RPGSkillComponent.h"

ARPGCharacterBase::ARPGCharacterBase()
{
    // 턴제 RPG는 매 프레임 연산이 거의 불필요하므로 기본적으로 꺼둠
    PrimaryActorTick.bCanEverTick = false;

    StatComponent = CreateDefaultSubobject<URPGStatComponent>(TEXT("StatComponent"));
    SkillComponent = CreateDefaultSubobject<URPGSkillComponent>(TEXT("SkillComponent"));

    bIsDead = false;
}

void ARPGCharacterBase::TakeRPGDamage(int32 DamageAmount, FName DamageType)
{
    if (!StatComponent || bIsDead)
    {
        return;
    }

    StatComponent->CurrentHP = FMath::Max(0, StatComponent->CurrentHP - DamageAmount);

    UE_LOG(LogTemp, Warning, TEXT("[%s] Took %d damage (%s). Current HP: %d"),
        *GetName(), DamageAmount, *DamageType.ToString(), StatComponent->CurrentHP);

    if (StatComponent->CurrentHP <= 0)
    {
        bIsDead = true;
        UE_LOG(LogTemp, Warning, TEXT("[%s] has died."), *GetName());
    }
}