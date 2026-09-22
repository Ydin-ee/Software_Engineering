#include "BattleHUD.h"
#include "CommandMenu.h"
#include "TurnQueueUI.h"
#include "DamageFloatingText.h"
#include "PlayerCharacter.h"
#include "RPGMonsterBase.h"
#include "RPGStatComponent.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UBattleHUD::InitializeForBattle(ARPGBattleManager* InBattleManager)
{
    if (BoundBattleManager)
    {
        BoundBattleManager->OnCombatEvent.RemoveDynamic(this, &UBattleHUD::HandleCombatEvent);
        BoundBattleManager->OnBattleStateChanged.RemoveDynamic(this, &UBattleHUD::HandleBattleStateChanged);
    }

    BoundBattleManager = InBattleManager;

    if (!BoundBattleManager)
    {
        return;
    }

    BoundBattleManager->OnCombatEvent.AddDynamic(this, &UBattleHUD::HandleCombatEvent);
    BoundBattleManager->OnBattleStateChanged.AddDynamic(this, &UBattleHUD::HandleBattleStateChanged);

    if (TurnQueueWidget)
    {
        TurnQueueWidget->BindToBattleManager(BoundBattleManager);
    }

    RefreshStatusText();
}

void UBattleHUD::ShowCommandMenu(ARPGPlayerController* InController, APlayerCharacter* InActingCharacter, const TArray<AActor*>& InPossibleTargets)
{
    if (CommandMenuWidget)
    {
        CommandMenuWidget->BeginSelection(InController, InActingCharacter, InPossibleTargets);
    }
}

void UBattleHUD::HideCommandMenu()
{
    if (CommandMenuWidget)
    {
        CommandMenuWidget->ClearSelection();
    }
}

void UBattleHUD::HandleCombatEvent(AActor* Target, int32 DamageAmount, bool bMissed)
{
    RefreshStatusText();

    if (!DamageTextLayer || !DamageFloatingTextClass || !Target)
    {
        return;
    }

    APlayerController* OwningPC = GetOwningPlayer();
    if (!OwningPC)
    {
        return;
    }

    UDamageFloatingText* FloatingText = CreateWidget<UDamageFloatingText>(OwningPC, DamageFloatingTextClass);
    if (!FloatingText)
    {
        return;
    }

    FloatingText->SetDamageInfo(DamageAmount, bMissed);

    if (UCanvasPanelSlot* CanvasSlot = DamageTextLayer->AddChildToCanvas(FloatingText))
    {
        FVector2D ScreenPosition;
        UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(OwningPC, Target->GetActorLocation(), ScreenPosition, false);
        CanvasSlot->SetPosition(ScreenPosition);
    }
}

void UBattleHUD::HandleBattleStateChanged(ERPGBattleState NewState)
{
    if (!BattleResultText)
    {
        return;
    }

    if (NewState == ERPGBattleState::State_BattleEnd)
    {
        const bool bAnyPlayerAlive = BoundBattleManager && BoundBattleManager->PlayerParty.ContainsByPredicate(
            [](APlayerCharacter* P) { return P && !P->bIsDead; });

        BattleResultText->SetText(FText::FromString(bAnyPlayerAlive ? TEXT("VICTORY") : TEXT("DEFEAT")));
        BattleResultText->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        BattleResultText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UBattleHUD::RefreshStatusText()
{
    if (!BoundBattleManager)
    {
        return;
    }

    if (PartyStatusText)
    {
        FString Lines;
        for (APlayerCharacter* Player : BoundBattleManager->PlayerParty)
        {
            if (Player && Player->StatComponent)
            {
                Lines += FString::Printf(TEXT("%s HP: %d/%d\n"),
                    *Player->GetName(), Player->StatComponent->CurrentHP, Player->StatComponent->MonsterStat.MaxHP);
            }
        }
        PartyStatusText->SetText(FText::FromString(Lines));
    }

    if (EnemyStatusText)
    {
        FString Lines;
        for (ARPGMonsterBase* Monster : BoundBattleManager->EnemyParty)
        {
            if (Monster && Monster->StatComponent)
            {
                Lines += FString::Printf(TEXT("%s HP: %d/%d\n"),
                    *Monster->GetName(), Monster->StatComponent->CurrentHP, Monster->StatComponent->MonsterStat.MaxHP);
            }
        }
        EnemyStatusText->SetText(FText::FromString(Lines));
    }
}
