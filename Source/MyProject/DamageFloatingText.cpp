#include "DamageFloatingText.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UDamageFloatingText::SetDamageInfo(int32 DamageAmount, bool bMissed)
{
    if (!DamageText)
    {
        return;
    }

    DamageText->SetText(bMissed ? FText::FromString(TEXT("MISS")) : FText::AsNumber(DamageAmount));
}

void UDamageFloatingText::NativeConstruct()
{
    Super::NativeConstruct();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(LifetimeTimerHandle, this, &UDamageFloatingText::HandleLifetimeExpired, LifetimeSeconds, false);
    }
}

void UDamageFloatingText::HandleLifetimeExpired()
{
    RemoveFromParent();
}
