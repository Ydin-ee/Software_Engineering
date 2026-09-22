#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnQueueUI.generated.h"

class UVerticalBox;
class ARPGBattleManager;
class ARPGCharacterBase;

// Grayboxing turn-order readout: one text row per queued combatant, current turn marked.
// Assign a UVerticalBox named "QueueContainer" in the WBP subclass to see rows appear.
UCLASS()
class UTurnQueueUI : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidgetOptional))
    UVerticalBox* QueueContainer;

    // Subscribes to the battle manager's turn-queue delegate and does an initial refresh.
    UFUNCTION(BlueprintCallable, Category = "RPG|UI")
    void BindToBattleManager(ARPGBattleManager* InBattleManager);

    UFUNCTION(BlueprintCallable, Category = "RPG|UI")
    void RefreshQueue(const TArray<ARPGCharacterBase*>& Queue, int32 CurrentIndex);

protected:
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleTurnQueueChanged();

    UPROPERTY()
    ARPGBattleManager* BoundBattleManager;
};
