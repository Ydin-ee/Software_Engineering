#include "RPGPlayerController.h"
#include "RPGBattleManager.h"
#include "PlayerCharacter.h"
#include "BattleHUD.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

void ARPGPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (IsLocalPlayerController())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
            {
                Subsystem->AddMappingContext(CurrentContext, 0);
            }
        }
    }
}

void ARPGPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!BattleHUDClass)
    {
        return;
    }

    BattleHUDWidget = CreateWidget<UBattleHUD>(this, BattleHUDClass);
    if (!BattleHUDWidget)
    {
        return;
    }

    BattleHUDWidget->AddToViewport();

    if (ARPGBattleManager* FoundBattleManager = Cast<ARPGBattleManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ARPGBattleManager::StaticClass())))
    {
        BattleHUDWidget->InitializeForBattle(FoundBattleManager);
    }
}

void ARPGPlayerController::BeginSkillSelection(ARPGBattleManager* InBattleManager, APlayerCharacter* InActingCharacter, const TArray<AActor*>& InPossibleTargets)
{
    ActiveBattleManager = InBattleManager;
    ActingCharacter = InActingCharacter;
    PossibleTargets = InPossibleTargets;
    SelectedSkillID = NAME_None;
    SelectedTarget = nullptr;
    bHasChangedCoreThisTurn = false;

    UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Your turn - select a skill and target (%d possible targets)."),
        PossibleTargets.Num());

    if (BattleHUDWidget)
    {
        BattleHUDWidget->ShowCommandMenu(this, InActingCharacter, InPossibleTargets);
    }
}

void ARPGPlayerController::SelectSkill(FName SkillID)
{
    SelectedSkillID = SkillID;
    UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Skill selected: %s"), *SkillID.ToString());
}

void ARPGPlayerController::SelectTarget(AActor* Target)
{
    SelectedTarget = Target;
    UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Target selected: %s"), Target ? *Target->GetName() : TEXT("None"));
}

void ARPGPlayerController::RequestChangeCore(FName NewCoreID)
{
    if (bHasChangedCoreThisTurn)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Core already changed this turn."));
        return;
    }

    if (!ActingCharacter)
    {
        return;
    }

    ActingCharacter->ChangeCore(NewCoreID);
    bHasChangedCoreThisTurn = true;

    // Refresh the command menu so the newly granted skill list shows immediately;
    // the player still picks a skill/target normally afterward.
    if (BattleHUDWidget)
    {
        BattleHUDWidget->ShowCommandMenu(this, ActingCharacter, PossibleTargets);
    }
}

void ARPGPlayerController::ConfirmAction()
{
    if (!ActiveBattleManager || !ActingCharacter || SelectedSkillID.IsNone() || !SelectedTarget)
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayerController] Cannot confirm action - skill/target not fully selected."));
        return;
    }

    ARPGBattleManager* BattleManager = ActiveBattleManager;
    APlayerCharacter* Actor = ActingCharacter;
    FName SkillID = SelectedSkillID;
    AActor* Target = SelectedTarget;

    // Clear turn state before submitting so a stray double-confirm can't resubmit.
    ActiveBattleManager = nullptr;
    ActingCharacter = nullptr;
    SelectedSkillID = NAME_None;
    SelectedTarget = nullptr;

    if (BattleHUDWidget)
    {
        BattleHUDWidget->HideCommandMenu();
    }

    BattleManager->SubmitAction(Actor, SkillID, Target);
}
