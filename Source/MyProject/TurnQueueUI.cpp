#include "TurnQueueUI.h"
#include "RPGBattleManager.h"
#include "RPGCharacterBase.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

void UTurnQueueUI::BindToBattleManager(ARPGBattleManager* InBattleManager)
{
    if (BoundBattleManager)
    {
        BoundBattleManager->OnTurnQueueChanged.RemoveDynamic(this, &UTurnQueueUI::HandleTurnQueueChanged);
    }

    BoundBattleManager = InBattleManager;

    if (BoundBattleManager)
    {
        BoundBattleManager->OnTurnQueueChanged.AddDynamic(this, &UTurnQueueUI::HandleTurnQueueChanged);
        HandleTurnQueueChanged();
    }
}

void UTurnQueueUI::HandleTurnQueueChanged()
{
    if (BoundBattleManager)
    {
        RefreshQueue(BoundBattleManager->TurnQueue, BoundBattleManager->CurrentTurnIndex);
    }
}

void UTurnQueueUI::RefreshQueue(const TArray<ARPGCharacterBase*>& Queue, int32 CurrentIndex)
{
    if (!QueueContainer)
    {
        return;
    }

    QueueContainer->ClearChildren();

    for (int32 Index = 0; Index < Queue.Num(); ++Index)
    {
        ARPGCharacterBase* Combatant = Queue[Index];
        if (!Combatant)
        {
            continue;
        }

        UTextBlock* Row = NewObject<UTextBlock>(this);
        const FString Prefix = (Index == CurrentIndex) ? TEXT("-> ") : TEXT("   ");
        // Data-table name (PC_DATA Name / MD_DATA MonsterName, numbered for duplicates) per FR-16;
        // the raw actor name only when RefreshQueue is driven without a bound battle manager.
        const FString DisplayName = BoundBattleManager
            ? BoundBattleManager->GetCombatantDisplayName(Combatant)
            : Combatant->GetName();
        Row->SetText(FText::FromString(Prefix + DisplayName));
        QueueContainer->AddChildToVerticalBox(Row);
    }
}

void UTurnQueueUI::NativeDestruct()
{
    if (BoundBattleManager)
    {
        BoundBattleManager->OnTurnQueueChanged.RemoveDynamic(this, &UTurnQueueUI::HandleTurnQueueChanged);
    }
    Super::NativeDestruct();
}
