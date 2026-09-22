#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RPGBattleManager.h"
#include "BattleHUD.generated.h"

class UTextBlock;
class UCanvasPanel;
class UCommandMenu;
class UTurnQueueUI;
class UDamageFloatingText;
class ARPGPlayerController;
class APlayerCharacter;

// Grayboxing battle screen: composes CommandMenu + TurnQueueUI, shows party/enemy
// HP readouts, and spawns floating damage numbers - just enough to verify that
// battle data actually reaches the right widget.
UCLASS()
class UBattleHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* PartyStatusText;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* EnemyStatusText;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* BattleResultText;

    UPROPERTY(meta = (BindWidgetOptional))
    UCommandMenu* CommandMenuWidget;

    UPROPERTY(meta = (BindWidgetOptional))
    UTurnQueueUI* TurnQueueWidget;

    // A CanvasPanel so floating damage text can be placed at an arbitrary screen position.
    UPROPERTY(meta = (BindWidgetOptional))
    UCanvasPanel* DamageTextLayer;

    UPROPERTY(EditDefaultsOnly, Category = "RPG|UI")
    TSubclassOf<UDamageFloatingText> DamageFloatingTextClass;

    UFUNCTION(BlueprintCallable, Category = "RPG|UI")
    void InitializeForBattle(ARPGBattleManager* InBattleManager);

    UFUNCTION(BlueprintCallable, Category = "RPG|UI")
    void ShowCommandMenu(ARPGPlayerController* InController, APlayerCharacter* InActingCharacter, const TArray<AActor*>& InPossibleTargets);

    UFUNCTION(BlueprintCallable, Category = "RPG|UI")
    void HideCommandMenu();

private:
    UFUNCTION()
    void HandleCombatEvent(AActor* Target, int32 DamageAmount, bool bMissed);

    UFUNCTION()
    void HandleBattleStateChanged(ERPGBattleState NewState);

    void RefreshStatusText();

    UPROPERTY()
    ARPGBattleManager* BoundBattleManager;
};
