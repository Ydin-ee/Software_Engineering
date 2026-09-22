#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPGBattleManager.generated.h"

class ARPGCharacterBase;
class APlayerCharacter;
class ARPGMonsterBase;
class URPGSkillComponent;
class ABattleArena;
class AFieldEncounter;

UENUM(BlueprintType)
enum class ERPGBattleState : uint8
{
    State_Wait          UMETA(DisplayName = "Wait"),
    State_CalculateTurn UMETA(DisplayName = "Calculate Turn"),
    State_PlayerTurn    UMETA(DisplayName = "Player Turn"),
    State_EnemyTurn     UMETA(DisplayName = "Enemy Turn"),
    State_BattleEnd     UMETA(DisplayName = "Battle End")
};

// UI hooks (UTurnQueueUI, UBattleHUD) bind to these instead of polling every frame.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnQueueChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleStateChanged, ERPGBattleState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatEvent, AActor*, Target, int32, DamageAmount, bool, bMissed);

UCLASS()
class MYPROJECT_API ARPGBattleManager : public AActor
{
    GENERATED_BODY()

public:
    ARPGBattleManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RPG|Battle")
    ERPGBattleState CurrentState;

    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void SetBattleState(ERPGBattleState NewState);

    // --- Turn queue ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Battle")
    TArray<APlayerCharacter*> PlayerParty;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Battle")
    TArray<ARPGMonsterBase*> EnemyParty;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Battle")
    TArray<ARPGCharacterBase*> TurnQueue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Battle")
    int32 CurrentTurnIndex;

    // Re-gathers every APlayerCharacter in the world (so a companion who joined mid-game
    // is picked up automatically), sets EnemyParty to Enemies, and starts a fresh battle.
    // Used both for the initial graybox battle (BeginPlay) and for repeatable field
    // encounters, so this BattleManager can be reused across many battles.
    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void StartBattle(const TArray<ARPGMonsterBase*>& Enemies);

    // Set true on a boss-room BattleManager instance. On victory, ClearedStage is recorded
    // (via RPGSaveGame) and any companion whose join condition is now met is spawned in.
    UPROPERTY(EditAnywhere, Category = "RPG|Battle")
    bool bIsBossBattle = false;

    UPROPERTY(EditAnywhere, Category = "RPG|Battle")
    int32 BossStageNumber = 0;

    // --- Field encounters (real-time exploration -> turn-based arena) ---

    // Reusable hidden arena every encounter teleports into. Assign explicitly, or leave
    // null to auto-find the first ABattleArena in the level on first use.
    UPROPERTY(EditAnywhere, Category = "RPG|Battle")
    ABattleArena* Arena;

    UPROPERTY(EditAnywhere, Category = "RPG|Battle")
    float FadeDuration = 0.5f;

    // Called by AFieldEncounter when the player touches it: looks up Stage/Battle in
    // Level_Encounter_Table, spawns the resulting monsters into the Arena, and teleports
    // the party there behind a camera fade. On victory the party is teleported back to
    // its field position and SourceEncounter is destroyed for good.
    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void BeginFieldEncounter(FName StageID, FName BattleID, AFieldEncounter* SourceEncounter);

    // Called by ARPGPlayerController::ConfirmAction and AMonsterAIController::TakeBattleTurn
    // once the acting combatant has chosen a skill + target. Executes it (if valid) and
    // advances the turn queue either way, so a bad/locked choice never stalls the battle.
    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    void SubmitAction(ARPGCharacterBase* Actor, FName SkillID, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    TArray<AActor*> GetLivingPlayerTargets() const;

    UFUNCTION(BlueprintCallable, Category = "RPG|Battle")
    TArray<AActor*> GetLivingEnemyTargets() const;

    // --- UI hooks ---

    UPROPERTY(BlueprintAssignable, Category = "RPG|Battle")
    FOnTurnQueueChanged OnTurnQueueChanged;

    UPROPERTY(BlueprintAssignable, Category = "RPG|Battle")
    FOnBattleStateChanged OnBattleStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "RPG|Battle")
    FOnCombatEvent OnCombatEvent;

private:
    void GatherPlayerParty();
    void BindSkillResolvedDelegate(URPGSkillComponent* SkillComponent);
    void BuildTurnQueue();
    void StartCurrentActorTurn();
    void AdvanceTurn();
    bool CheckBattleEnd();
    void HandleVictory();
    void HandleStageCleared(int32 ClearedStageNumber);
    void SpawnCompanion(FName CompanionCharacterID);

    void TeleportToArenaAndStartBattle(TArray<ARPGMonsterBase*> SpawnedEnemies);
    void ReturnPartyToField();
    void FinishReturnToField();

    // Relays every party member's SkillComponent::OnSkillResolved into OnCombatEvent,
    // so UI only has to bind to the battle manager once instead of per-combatant.
    UFUNCTION()
    void HandleSkillResolved(AActor* Target, int32 DamageAmount, bool bMissed);

    UPROPERTY()
    TMap<APlayerCharacter*, FTransform> FieldReturnTransforms;

    UPROPERTY()
    AFieldEncounter* PendingSourceEncounter;

    FTimerHandle TransitionTimerHandle;
};
