#include "RPGBattleManager.h"
#include "RPGCharacterBase.h"
#include "PlayerCharacter.h"
#include "RPGMonsterBase.h"
#include "RPGStatComponent.h"
#include "RPGSkillComponent.h"
#include "RPGPlayerController.h"
#include "MonsterAIController.h"
#include "RPGDataManager.h"
#include "BattleArena.h"
#include "FieldEncounter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

ARPGBattleManager::ARPGBattleManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentState = ERPGBattleState::State_Wait;
    CurrentTurnIndex = 0;
}

void ARPGBattleManager::BeginPlay()
{
    Super::BeginPlay();

    // Grayboxing: start a battle against whatever monsters are already placed in the
    // level, rather than requiring a field encounter trigger to exist yet. Opt-in only.
    if (!bAutoStartWithPlacedMonsters)
    {
        return;
    }

    TArray<AActor*> FoundMonsters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARPGMonsterBase::StaticClass(), FoundMonsters);

    TArray<ARPGMonsterBase*> InitialEnemies;
    for (AActor* Actor : FoundMonsters)
    {
        if (ARPGMonsterBase* Monster = Cast<ARPGMonsterBase>(Actor))
        {
            InitialEnemies.Add(Monster);
        }
    }

    // Only auto-start a battle if monsters were placed directly in the level; a level
    // that relies purely on AFieldEncounter triggers just waits for the first one instead.
    if (InitialEnemies.Num() > 0)
    {
        StartBattle(InitialEnemies);
    }
}

void ARPGBattleManager::BeginFieldEncounter(FName StageID, FName BattleID, AFieldEncounter* SourceEncounter)
{
    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;
    if (!DataManager)
    {
        return;
    }

    FEncounterRow EncounterRow;
    if (!DataManager->GetEncounterData(StageID, BattleID, EncounterRow))
    {
        UE_LOG(LogTemp, Error, TEXT("[BattleManager] No encounter data for %s/%s."), *StageID.ToString(), *BattleID.ToString());
        return;
    }

    if (!Arena)
    {
        Arena = Cast<ABattleArena>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleArena::StaticClass()));
    }
    if (!Arena)
    {
        UE_LOG(LogTemp, Error, TEXT("[BattleManager] No ABattleArena found in the level."));
        return;
    }

    PendingSourceEncounter = SourceEncounter;

    // One BattleManager serves every encounter, so boss-ness is per battle: take it from
    // the trigger and reset it for ordinary encounters so a previous boss flag can't linger.
    bIsBossBattle = SourceEncounter && SourceEncounter->bIsBossEncounter;
    BossStageNumber = bIsBossBattle ? SourceEncounter->BossStageNumber : 0;

    TArray<FName> MobIDs;
    TArray<int32> Counts;
    URPGDataManager::ParseEncounterMobList(EncounterRow, MobIDs, Counts);

    TArray<ARPGMonsterBase*> SpawnedEnemies;
    int32 SlotIndex = 0;
    for (int32 GroupIndex = 0; GroupIndex < MobIDs.Num(); ++GroupIndex)
    {
        const FName MobID = MobIDs[GroupIndex];
        const int32 Count = Counts[GroupIndex];

        for (int32 i = 0; i < Count; ++i)
        {
            const FTransform SpawnTransform = Arena->GetEnemySlotTransform(SlotIndex++);
            ARPGMonsterBase* NewMonster = GetWorld()->SpawnActorDeferred<ARPGMonsterBase>(ARPGMonsterBase::StaticClass(), SpawnTransform);
            if (NewMonster)
            {
                NewMonster->MonsterID = MobID;
                NewMonster->FinishSpawning(SpawnTransform);
                SpawnedEnemies.Add(NewMonster);
            }
        }
    }

    if (SpawnedEnemies.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[BattleManager] Encounter %s/%s produced no monsters."), *StageID.ToString(), *BattleID.ToString());
        return;
    }

    // Cache field positions for every current player actor before moving anyone.
    FieldReturnTransforms.Reset();
    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), FoundPlayers);
    for (AActor* Actor : FoundPlayers)
    {
        if (APlayerCharacter* Player = Cast<APlayerCharacter>(Actor))
        {
            FieldReturnTransforms.Add(Player, Player->GetActorTransform());
        }
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeDuration, FLinearColor::Black, false, true);
        }
    }

    FTimerDelegate TimerDel = FTimerDelegate::CreateUObject(this, &ARPGBattleManager::TeleportToArenaAndStartBattle, SpawnedEnemies);
    GetWorldTimerManager().SetTimer(TransitionTimerHandle, TimerDel, FadeDuration, false);
}

void ARPGBattleManager::TeleportToArenaAndStartBattle(TArray<ARPGMonsterBase*> SpawnedEnemies)
{
    int32 SlotIndex = 0;
    for (const TPair<APlayerCharacter*, FTransform>& Pair : FieldReturnTransforms)
    {
        if (Pair.Key)
        {
            Pair.Key->SetActorTransform(Arena->GetPlayerSlotTransform(SlotIndex++));
        }
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeDuration, FLinearColor::Black, false, true);
        }

        // Block field movement/look input while the turn-based battle UI is up -
        // DisableInput only blocks the pawn's InputComponent, not UMG widget clicks.
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerPawn->DisableInput(PC);
        }

        // DisableInput doesn't touch cursor visibility or input mode - without this the
        // mouse stays hidden/locked to the viewport and CommandMenu buttons can't be clicked.
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeGameAndUI());
    }

    StartBattle(SpawnedEnemies);
}

void ARPGBattleManager::ReturnPartyToField()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeDuration, FLinearColor::Black, false, true);
        }
    }

    FTimerDelegate TimerDel = FTimerDelegate::CreateUObject(this, &ARPGBattleManager::FinishReturnToField);
    GetWorldTimerManager().SetTimer(TransitionTimerHandle, TimerDel, FadeDuration, false);
}

void ARPGBattleManager::FinishReturnToField()
{
    for (const TPair<APlayerCharacter*, FTransform>& Pair : FieldReturnTransforms)
    {
        if (Pair.Key)
        {
            Pair.Key->SetActorTransform(Pair.Value);
        }
    }
    FieldReturnTransforms.Reset();

    if (PendingSourceEncounter)
    {
        PendingSourceEncounter->Destroy();
        PendingSourceEncounter = nullptr;
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeDuration, FLinearColor::Black, false, true);
        }

        // Restore field movement/look input now that the party is back and the battle UI is gone.
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerPawn->EnableInput(PC);
        }

        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }

    // The party is back on the field, so the battle is over for good (FR-02/FR-17). Drop
    // the stale queue (its monsters were destroyed in HandleVictory) and leave BattleEnd so
    // UBattleHUD can hide the VICTORY text and the turn-order list. This runs only on
    // victory; defeat stays in State_BattleEnd (game-over flow is out of scope).
    TurnQueue.Reset();
    CurrentTurnIndex = 0;
    OnTurnQueueChanged.Broadcast();

    SetBattleState(ERPGBattleState::State_Wait);
}

void ARPGBattleManager::StartBattle(const TArray<ARPGMonsterBase*>& Enemies)
{
    // Always re-gather the player party fresh so a companion who joined mid-game
    // (see HandleStageCleared/SpawnCompanion) is included in every subsequent battle.
    GatherPlayerParty();

    EnemyParty = Enemies;
    for (ARPGMonsterBase* Monster : EnemyParty)
    {
        if (Monster)
        {
            BindSkillResolvedDelegate(Monster->SkillComponent);
        }
    }

    TurnQueue.Reset();
    CurrentTurnIndex = 0;

    SetBattleState(ERPGBattleState::State_CalculateTurn);
}

void ARPGBattleManager::GatherPlayerParty()
{
    PlayerParty.Reset();

    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), FoundPlayers);
    for (AActor* Actor : FoundPlayers)
    {
        if (APlayerCharacter* Player = Cast<APlayerCharacter>(Actor))
        {
            PlayerParty.Add(Player);
            BindSkillResolvedDelegate(Player->SkillComponent);
        }
    }
}

void ARPGBattleManager::BindSkillResolvedDelegate(URPGSkillComponent* SkillComponent)
{
    // AddDynamic doesn't dedupe, and this BattleManager can be reused across many
    // battles for the same persistent player characters - guard against double-binding.
    if (SkillComponent && !SkillComponent->OnSkillResolved.IsAlreadyBound(this, &ARPGBattleManager::HandleSkillResolved))
    {
        SkillComponent->OnSkillResolved.AddDynamic(this, &ARPGBattleManager::HandleSkillResolved);
    }
}

void ARPGBattleManager::HandleSkillResolved(AActor* Target, int32 DamageAmount, bool bMissed)
{
    OnCombatEvent.Broadcast(Target, DamageAmount, bMissed);
}

void ARPGBattleManager::SetBattleState(ERPGBattleState NewState)
{
    CurrentState = NewState;
    OnBattleStateChanged.Broadcast(CurrentState);

    switch (CurrentState)
    {
    case ERPGBattleState::State_Wait:
        UE_LOG(LogTemp, Warning, TEXT("[BattleManager] Waiting..."));
        break;

    case ERPGBattleState::State_CalculateTurn:
        UE_LOG(LogTemp, Warning, TEXT("[BattleManager] Calculating Turn..."));
        BuildTurnQueue();
        break;

    case ERPGBattleState::State_PlayerTurn:
        UE_LOG(LogTemp, Warning, TEXT("[BattleManager] Player Turn!"));
        break;

    case ERPGBattleState::State_EnemyTurn:
        UE_LOG(LogTemp, Error, TEXT("[BattleManager] Enemy Turn!"));
        break;

    case ERPGBattleState::State_BattleEnd:
        UE_LOG(LogTemp, Warning, TEXT("[BattleManager] Battle End."));
        break;
    }
}

void ARPGBattleManager::BuildTurnQueue()
{
    TurnQueue.Reset();

    for (APlayerCharacter* Player : PlayerParty)
    {
        if (Player && !Player->bIsDead)
        {
            TurnQueue.Add(Player);
        }
    }
    for (ARPGMonsterBase* Monster : EnemyParty)
    {
        if (Monster && !Monster->bIsDead)
        {
            TurnQueue.Add(Monster);
        }
    }

    // Highest SPD acts first this round.
    // StableSort keeps the gather order (players first) for equal SPD instead of leaving ties arbitrary.
    TurnQueue.StableSort([](const ARPGCharacterBase& A, const ARPGCharacterBase& B)
    {
        const int32 SpdA = A.StatComponent ? A.StatComponent->MonsterStat.SPD : 0;
        const int32 SpdB = B.StatComponent ? B.StatComponent->MonsterStat.SPD : 0;
        return SpdA > SpdB;
    });

    CurrentTurnIndex = 0;

    if (CheckBattleEnd())
    {
        return;
    }

    StartCurrentActorTurn();
}

void ARPGBattleManager::StartCurrentActorTurn()
{
    if (CheckBattleEnd())
    {
        return;
    }

    // Skip anyone who died since the queue was built for this round.
    while (TurnQueue.IsValidIndex(CurrentTurnIndex) &&
        (!TurnQueue[CurrentTurnIndex] || TurnQueue[CurrentTurnIndex]->bIsDead))
    {
        ++CurrentTurnIndex;
    }

    if (!TurnQueue.IsValidIndex(CurrentTurnIndex))
    {
        // Round finished - recalculate for the next one.
        SetBattleState(ERPGBattleState::State_CalculateTurn);
        return;
    }

    OnTurnQueueChanged.Broadcast();

    ARPGCharacterBase* Actor = TurnQueue[CurrentTurnIndex];

    if (APlayerCharacter* Player = Cast<APlayerCharacter>(Actor))
    {
        SetBattleState(ERPGBattleState::State_PlayerTurn);
        UE_LOG(LogTemp, Warning, TEXT("[BattleManager] -> %s's turn."), *Player->GetName());

        // A single player controller commands whichever party member's turn it is - allies placed
        // in the level (e.g. PC_002) have no controller of their own, so Player->GetController() would skip them.
        if (ARPGPlayerController* PC = Cast<ARPGPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->BeginSkillSelection(this, Player, GetLivingEnemyTargets());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[BattleManager] No ARPGPlayerController found, skipping %s's turn."), *Player->GetName());
            AdvanceTurn();
        }
    }
    else if (ARPGMonsterBase* Monster = Cast<ARPGMonsterBase>(Actor))
    {
        SetBattleState(ERPGBattleState::State_EnemyTurn);
        UE_LOG(LogTemp, Error, TEXT("[BattleManager] -> %s's turn."), *Monster->GetName());

        if (AMonsterAIController* AIC = Cast<AMonsterAIController>(Monster->GetController()))
        {
            AIC->TakeBattleTurn(this);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[BattleManager] %s has no AMonsterAIController, skipping turn."), *Monster->GetName());
            AdvanceTurn();
        }
    }
}

void ARPGBattleManager::SubmitAction(ARPGCharacterBase* Actor, FName SkillID, AActor* Target)
{
    if (!TurnQueue.IsValidIndex(CurrentTurnIndex) || TurnQueue[CurrentTurnIndex] != Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("[BattleManager] SubmitAction called out of turn, ignoring."));
        return;
    }

    if (Actor && Actor->SkillComponent && Target && !SkillID.IsNone())
    {
        Actor->SkillComponent->ExecuteSkill(Target, SkillID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[BattleManager] %s submitted an invalid action, turn wasted."),
            Actor ? *Actor->GetName() : TEXT("None"));
    }

    AdvanceTurn();
}

void ARPGBattleManager::AdvanceTurn()
{
    if (CheckBattleEnd())
    {
        return;
    }

    ++CurrentTurnIndex;
    StartCurrentActorTurn();
}

bool ARPGBattleManager::CheckBattleEnd()
{
    const bool bAnyPlayerAlive = PlayerParty.ContainsByPredicate([](APlayerCharacter* P) { return P && !P->bIsDead; });
    const bool bAnyEnemyAlive = EnemyParty.ContainsByPredicate([](ARPGMonsterBase* M) { return M && !M->bIsDead; });

    if (!bAnyPlayerAlive || !bAnyEnemyAlive)
    {
        if (bAnyPlayerAlive)
        {
            HandleVictory();
        }
        // Defeat: leaves EnemyParty as-is and just reports the state; the actual
        // game-over flow (restart / load / title) is a separate UI task, not built yet.

        SetBattleState(ERPGBattleState::State_BattleEnd);
        UE_LOG(LogTemp, Warning, TEXT("[BattleManager] %s"), bAnyPlayerAlive ? TEXT("Victory!") : TEXT("Defeat..."));
        return true;
    }
    return false;
}

void ARPGBattleManager::HandleVictory()
{
    // Every surviving party member gets the full DropEXP of each defeated monster.
    int32 TotalDropExp = 0;
    for (ARPGMonsterBase* Monster : EnemyParty)
    {
        if (Monster && Monster->bIsDead && Monster->StatComponent)
        {
            TotalDropExp += Monster->StatComponent->MonsterStat.DropEXP;
        }
    }

    if (TotalDropExp > 0)
    {
        for (APlayerCharacter* Player : PlayerParty)
        {
            if (Player && !Player->bIsDead)
            {
                Player->GainExp(TotalDropExp);
            }
        }
    }

    // Defeated field monsters are permanently gone (no respawning for now).
    for (ARPGMonsterBase* Monster : EnemyParty)
    {
        if (Monster)
        {
            Monster->Destroy();
        }
    }
    EnemyParty.Reset();

    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;

    // Every victory persists the equipped core (FR-15). ClearedStage 0 here is harmless:
    // SaveProgress never lets it drop below the value already on disk.
    if (DataManager)
    {
        DataManager->SaveProgress(GetPlayerPawnEquippedCoreID(), 0);
    }

    if (bIsBossBattle)
    {
        HandleStageCleared(BossStageNumber);
    }

    // Must run before ReturnPartyToField(), which clears PendingSourceEncounter.
    if (DataManager && IsTutorialBattle())
    {
        DataManager->SaveTutorialCleared();
    }

    if (PendingSourceEncounter)
    {
        ReturnPartyToField();
    }
}

bool ARPGBattleManager::IsTutorialBattle() const
{
    return PendingSourceEncounter && PendingSourceEncounter->StageID == FName(TEXT("Tutorial"));
}

FName ARPGBattleManager::GetPlayerPawnEquippedCoreID() const
{
    const APlayerCharacter* PlayerPawn = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    return PlayerPawn ? PlayerPawn->EquippedCoreID : NAME_None;
}

FString ARPGBattleManager::GetCombatantDisplayName(const ARPGCharacterBase* Combatant) const
{
    if (!Combatant)
    {
        return FString();
    }

    // MonsterStat.MonsterName holds MD_DATA MonsterName for monsters and PC_DATA Name for
    // players (copied in APlayerCharacter::InitializeCharacterStat).
    if (!Combatant->StatComponent || Combatant->StatComponent->MonsterStat.MonsterName.IsNone())
    {
        return Combatant->GetName();
    }

    FString DisplayName = Combatant->StatComponent->MonsterStat.MonsterName.ToString();

    if (const ARPGMonsterBase* Monster = Cast<ARPGMonsterBase>(Combatant))
    {
        int32 SameIDCount = 0;
        int32 Ordinal = 0;
        for (const ARPGMonsterBase* Other : EnemyParty)
        {
            if (Other && Other->MonsterID == Monster->MonsterID)
            {
                ++SameIDCount;
                if (Other == Monster)
                {
                    Ordinal = SameIDCount;
                }
            }
        }

        if (SameIDCount > 1 && Ordinal > 0)
        {
            DisplayName += FString::Printf(TEXT(" %d"), Ordinal);
        }
    }

    return DisplayName;
}

void ARPGBattleManager::HandleStageCleared(int32 ClearedStageNumber)
{
    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;
    if (DataManager)
    {
        DataManager->SaveProgress(GetPlayerPawnEquippedCoreID(), ClearedStageNumber);
    }

    // Companion join conditions are hardcoded to the two known join points for now
    // (no separate "party join" data table exists yet).
    FName JoiningCharacterID = NAME_None;
    if (ClearedStageNumber == 1)
    {
        JoiningCharacterID = TEXT("PC_002");
    }
    else if (ClearedStageNumber == 2)
    {
        JoiningCharacterID = TEXT("PC_003");
    }

    if (!JoiningCharacterID.IsNone())
    {
        SpawnCompanion(JoiningCharacterID);
    }
}

void ARPGBattleManager::SpawnCompanion(FName CompanionCharacterID)
{
    for (APlayerCharacter* Existing : PlayerParty)
    {
        if (Existing && Existing->CharacterID == CompanionCharacterID)
        {
            return; // Already joined.
        }
    }

    UWorld* World = GetWorld();
    if (!World || PlayerParty.Num() == 0 || !PlayerParty[0])
    {
        return;
    }

    const FTransform SpawnTransform(FRotator::ZeroRotator, PlayerParty[0]->GetActorLocation());

    APlayerCharacter* Companion = World->SpawnActorDeferred<APlayerCharacter>(APlayerCharacter::StaticClass(), SpawnTransform);
    if (!Companion)
    {
        UE_LOG(LogTemp, Error, TEXT("[BattleManager] Failed to spawn companion %s."), *CompanionCharacterID.ToString());
        return;
    }

    // Set before FinishSpawning so BeginPlay (which loads CharacterData) sees the right ID.
    Companion->CharacterID = CompanionCharacterID;
    Companion->FinishSpawning(SpawnTransform);

    PlayerParty.Add(Companion);
    BindSkillResolvedDelegate(Companion->SkillComponent);

    UE_LOG(LogTemp, Warning, TEXT("[BattleManager] %s has joined the party!"), *CompanionCharacterID.ToString());
}

TArray<AActor*> ARPGBattleManager::GetLivingPlayerTargets() const
{
    TArray<AActor*> Result;
    for (APlayerCharacter* Player : PlayerParty)
    {
        if (Player && !Player->bIsDead)
        {
            Result.Add(Player);
        }
    }
    return Result;
}

TArray<AActor*> ARPGBattleManager::GetLivingEnemyTargets() const
{
    TArray<AActor*> Result;
    for (ARPGMonsterBase* Monster : EnemyParty)
    {
        if (Monster && !Monster->bIsDead)
        {
            Result.Add(Monster);
        }
    }
    return Result;
}
