#include "BattleArena.h"

ABattleArena::ABattleArena()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

FTransform ABattleArena::GetPlayerSlotTransform(int32 Index) const
{
    if (PlayerSlotOffsets.Num() == 0)
    {
        return GetActorTransform();
    }

    const FVector Offset = PlayerSlotOffsets[Index % PlayerSlotOffsets.Num()];
    return FTransform(GetActorRotation(), GetActorLocation() + GetActorRotation().RotateVector(Offset));
}

FTransform ABattleArena::GetEnemySlotTransform(int32 Index) const
{
    if (EnemySlotOffsets.Num() == 0)
    {
        return GetActorTransform();
    }

    const FVector Offset = EnemySlotOffsets[Index % EnemySlotOffsets.Num()];
    return FTransform(GetActorRotation(), GetActorLocation() + GetActorRotation().RotateVector(Offset));
}
