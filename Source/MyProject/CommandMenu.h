#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "CommandMenu.generated.h"

class UVerticalBox;
class ARPGPlayerController;
class APlayerCharacter;
class UCommandMenu;

// Plain UButton that remembers which skill it represents, so one shared click
// handler can still tell buttons apart (UButton::OnClicked itself takes no params).
UCLASS()
class USkillCommandButton : public UButton
{
    GENERATED_BODY()

public:
    UPROPERTY()
    FName SkillID;

    UPROPERTY()
    UCommandMenu* OwningMenu = nullptr;

    UFUNCTION()
    void HandleClicked();
};

// Plain UButton that remembers which target actor it represents.
UCLASS()
class UTargetCommandButton : public UButton
{
    GENERATED_BODY()

public:
    UPROPERTY()
    AActor* TargetActor = nullptr;

    UPROPERTY()
    UCommandMenu* OwningMenu = nullptr;

    UFUNCTION()
    void HandleClicked();
};

// Plain UButton that remembers which Core it represents.
UCLASS()
class UCoreCommandButton : public UButton
{
    GENERATED_BODY()

public:
    UPROPERTY()
    FName CoreID;

    UPROPERTY()
    UCommandMenu* OwningMenu = nullptr;

    UFUNCTION()
    void HandleClicked();
};

// Grayboxing command UI: lists the acting character's skills (quest-locked ones
// disabled) then living targets. Picking a skill then a target submits the action.
UCLASS()
class UCommandMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidgetOptional))
    UVerticalBox* SkillListContainer;

    UPROPERTY(meta = (BindWidgetOptional))
    UVerticalBox* TargetListContainer;

    // Persona-style Core swap list (once per turn - see ARPGPlayerController::RequestChangeCore).
    UPROPERTY(meta = (BindWidgetOptional))
    UVerticalBox* CoreListContainer;

    // Called by UBattleHUD (via ARPGPlayerController::BeginSkillSelection) when it's this player's turn.
    UFUNCTION(BlueprintCallable, Category = "RPG|UI")
    void BeginSelection(ARPGPlayerController* InController, APlayerCharacter* InActingCharacter, const TArray<AActor*>& InPossibleTargets);

    UFUNCTION(BlueprintCallable, Category = "RPG|UI")
    void ClearSelection();

    void OnSkillChosen(FName SkillID);
    void OnTargetChosen(AActor* Target);
    void OnCoreChosen(FName CoreID);

private:
    void RebuildSkillList();
    void RebuildTargetList();
    void RebuildCoreList();

    UPROPERTY()
    ARPGPlayerController* BoundController;

    UPROPERTY()
    APlayerCharacter* ActingCharacter;

    UPROPERTY()
    TArray<AActor*> PossibleTargets;
};
