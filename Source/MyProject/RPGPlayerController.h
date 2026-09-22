#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RPGPlayerController.generated.h"

class ARPGBattleManager;
class APlayerCharacter;
class UBattleHUD;
class UInputMappingContext;

UCLASS()
class ARPGPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

public:
    // Enhanced Input mapping context(s) for field movement/camera (Move/Look/Jump).
    // Assign the project's IMC_Default (or equivalent) here in BP_RPGPlayerController.
    UPROPERTY(EditAnywhere, Category = "Input")
    TArray<UInputMappingContext*> DefaultMappingContexts;

public:
    // Assign a UBattleHUD (or WBP subclass) here to have it spawned automatically.
    UPROPERTY(EditDefaultsOnly, Category = "RPG|UI")
    TSubclassOf<UBattleHUD> BattleHUDClass;

    UPROPERTY(BlueprintReadOnly, Category = "RPG|UI")
    UBattleHUD* BattleHUDWidget;

    // Called by ARPGBattleManager when it becomes this player's turn.
    // Bind SelectSkill/SelectTarget/ConfirmAction to UI or input events in Blueprint.
    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void BeginSkillSelection(ARPGBattleManager* InBattleManager, APlayerCharacter* InActingCharacter, const TArray<AActor*>& InPossibleTargets);

    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void SelectSkill(FName SkillID);

    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void SelectTarget(AActor* Target);

    // Submits the currently selected skill + target to the battle manager.
    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void ConfirmAction();

    // Persona-style Core swap: once per turn, doesn't consume the turn itself - the
    // player can still pick and use a skill normally afterward.
    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void RequestChangeCore(FName NewCoreID);

    UPROPERTY(BlueprintReadOnly, Category = "RPG|Battle")
    bool bHasChangedCoreThisTurn = false;

    UFUNCTION(BlueprintPure, Category = "RPG|Battle")
    bool IsMyTurn() const { return ActiveBattleManager != nullptr; }

    UPROPERTY(BlueprintReadOnly, Category = "RPG|Battle")
    TArray<AActor*> PossibleTargets;

    UPROPERTY(BlueprintReadOnly, Category = "RPG|Battle")
    FName SelectedSkillID;

    UPROPERTY(BlueprintReadOnly, Category = "RPG|Battle")
    AActor* SelectedTarget;

private:
    UPROPERTY()
    ARPGBattleManager* ActiveBattleManager;

    UPROPERTY()
    APlayerCharacter* ActingCharacter;
};
