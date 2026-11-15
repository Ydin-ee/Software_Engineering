#include "CommandMenu.h"
#include "RPGPlayerController.h"
#include "PlayerCharacter.h"
#include "RPGCharacterBase.h"
#include "RPGBattleManager.h"
#include "RPGSkillComponent.h"
#include "RPGStatComponent.h"
#include "RPGDataManager.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// Korean UI copy is written as \u escapes so these ASCII source files stay free of
// code-page issues under MSVC. The English meaning is noted next to each literal.
static const TCHAR* LockedSuffix = TEXT(" (\uC7A0\uAE40)");   // " (Locked)"
static const TCHAR* NoneAffinity = TEXT("\uC5C6\uC74C");     // "None"

// Weak/Resist text from MD_DATA/PC_DATA as shown to the player: empty or "None" -> "None" (Korean).
static FString FormatAffinityForLabel(const FString& Affinity)
{
    const FString Trimmed = Affinity.TrimStartAndEnd();
    if (Trimmed.IsEmpty() || Trimmed.Equals(TEXT("None"), ESearchCase::IgnoreCase))
    {
        return NoneAffinity;
    }
    return Trimmed;
}

void USkillCommandButton::HandleClicked()
{
    if (OwningMenu)
    {
        OwningMenu->OnSkillChosen(SkillID);
    }
}

void UTargetCommandButton::HandleClicked()
{
    if (OwningMenu)
    {
        OwningMenu->OnTargetChosen(TargetActor);
    }
}

void UCoreCommandButton::HandleClicked()
{
    if (OwningMenu)
    {
        OwningMenu->OnCoreChosen(CoreID);
    }
}

void UCommandMenu::BeginSelection(ARPGPlayerController* InController, APlayerCharacter* InActingCharacter, const TArray<AActor*>& InPossibleTargets)
{
    BoundController = InController;
    ActingCharacter = InActingCharacter;
    PossibleTargets = InPossibleTargets;

    SetVisibility(ESlateVisibility::Visible);

    RebuildSkillList();
    RebuildTargetList();
    RebuildCoreList();
}

void UCommandMenu::ClearSelection()
{
    BoundController = nullptr;
    ActingCharacter = nullptr;
    PossibleTargets.Reset();

    if (SkillListContainer)
    {
        SkillListContainer->ClearChildren();
    }
    if (TargetListContainer)
    {
        TargetListContainer->ClearChildren();
    }
    if (CoreListContainer)
    {
        CoreListContainer->ClearChildren();
    }

    SetVisibility(ESlateVisibility::Collapsed);
}

void UCommandMenu::RebuildSkillList()
{
    if (!SkillListContainer || !ActingCharacter || !ActingCharacter->SkillComponent)
    {
        return;
    }

    SkillListContainer->ClearChildren();

    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;

    for (const FName& SkillID : ActingCharacter->SkillComponent->AvailableSkills)
    {
        const bool bUnlocked = DataManager && DataManager->IsSkillUnlocked(SkillID);
        // Unaffordable skills are greyed out like locked ones (label unchanged) so a pick
        // can never reach ExecuteSkill just to fail there and burn the turn.
        const bool bAffordable = ActingCharacter->SkillComponent->CanAffordSkill(SkillID);

        USkillCommandButton* Button = NewObject<USkillCommandButton>(this);
        Button->SkillID = SkillID;
        Button->OwningMenu = this;
        Button->SetIsEnabled(bUnlocked && bAffordable);

        // "<SkillName> [<Element>] SP 10" / "HP 16" (actual deduction), no cost text for None.
        // Everything comes from SK_DATA; the ID is only a fallback when the row is missing.
        FString LabelText = SkillID.ToString();
        FSkillDataRow SkillData;
        if (DataManager && DataManager->GetSkillData(SkillID, SkillData))
        {
            LabelText = FString::Printf(TEXT("%s [%s]"), *SkillData.SkillName.ToString(), *SkillData.ElementType.ToString());

            const bool bHasCost = SkillData.CostType == FName(TEXT("SP")) || SkillData.CostType == FName(TEXT("HP"));
            if (bHasCost)
            {
                LabelText += FString::Printf(TEXT(" %s %d"), *SkillData.CostType.ToString(),
                    ActingCharacter->SkillComponent->GetSkillCost(SkillData));
            }
        }
        if (!bUnlocked)
        {
            LabelText += LockedSuffix; // " (Locked)"
        }

        UTextBlock* Label = NewObject<UTextBlock>(this);
        Label->SetText(FText::FromString(LabelText));
        Button->AddChild(Label);

        Button->OnClicked.AddDynamic(Button, &USkillCommandButton::HandleClicked);
        SkillListContainer->AddChildToVerticalBox(Button);
    }
}

void UCommandMenu::RebuildTargetList()
{
    if (!TargetListContainer)
    {
        return;
    }

    TargetListContainer->ClearChildren();

    const ARPGBattleManager* BattleManager = BoundController ? BoundController->GetActiveBattleManager() : nullptr;

    for (AActor* Target : PossibleTargets)
    {
        if (!Target)
        {
            continue;
        }

        UTargetCommandButton* Button = NewObject<UTargetCommandButton>(this);
        Button->TargetActor = Target;
        Button->OwningMenu = this;

        // "<MonsterName [n]> HP <cur>/<max> Weak:<Weak> Resist:<Resist>" from MD_DATA via the
        // target's StatComponent (NFR-08). Non-RPG actors keep the raw actor name.
        FString LabelText = Target->GetName();
        if (const ARPGCharacterBase* TargetCharacter = Cast<ARPGCharacterBase>(Target))
        {
            if (TargetCharacter->StatComponent)
            {
                const FMonsterDataRow& Stat = TargetCharacter->StatComponent->MonsterStat;
                const FString DisplayName = BattleManager
                    ? BattleManager->GetCombatantDisplayName(TargetCharacter)
                    : Stat.MonsterName.ToString();

                LabelText = FString::Printf(TEXT("%s HP %d/%d \uC57D\uC810:%s \uB0B4\uC131:%s"), // "<name> HP cur/max Weak:<w> Resist:<r>"
                    *DisplayName,
                    TargetCharacter->StatComponent->CurrentHP, Stat.MaxHP,
                    *FormatAffinityForLabel(Stat.Weak),
                    *FormatAffinityForLabel(Stat.Resist));
            }
        }

        UTextBlock* Label = NewObject<UTextBlock>(this);
        Label->SetText(FText::FromString(LabelText));
        Button->AddChild(Label);

        Button->OnClicked.AddDynamic(Button, &UTargetCommandButton::HandleClicked);
        TargetListContainer->AddChildToVerticalBox(Button);
    }
}

void UCommandMenu::RebuildCoreList()
{
    if (!CoreListContainer)
    {
        return;
    }

    CoreListContainer->ClearChildren();

    // Once already used this turn, the whole section is pointless - leave it empty.
    if (BoundController && BoundController->bHasChangedCoreThisTurn)
    {
        return;
    }

    // Core swap is only for characters that start with a core (PC_001, FR-10).
    // Companions (EquippedCoreID == None) keep their SkillPool, so hide the list for them.
    if (!ActingCharacter || ActingCharacter->EquippedCoreID.IsNone())
    {
        return;
    }

    UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    URPGDataManager* DataManager = GI ? GI->GetSubsystem<URPGDataManager>() : nullptr;
    if (!DataManager)
    {
        return;
    }

    TArray<FName> CoreIDs;
    DataManager->GetAllCoreIDs(CoreIDs);

    for (const FName& CoreID : CoreIDs)
    {
        const bool bUnlocked = DataManager->IsCoreUnlocked(CoreID);

        UCoreCommandButton* Button = NewObject<UCoreCommandButton>(this);
        Button->CoreID = CoreID;
        Button->OwningMenu = this;
        Button->SetIsEnabled(bUnlocked);

        // CD_DATA CoreName, with the ID only as a fallback when the row has no name.
        FString LabelText = CoreID.ToString();
        FCoreDataRow CoreData;
        if (DataManager->GetCoreData(CoreID, CoreData) && !CoreData.CoreName.IsNone())
        {
            LabelText = CoreData.CoreName.ToString();
        }
        if (!bUnlocked)
        {
            LabelText += LockedSuffix; // " (Locked)"
        }

        UTextBlock* Label = NewObject<UTextBlock>(this);
        Label->SetText(FText::FromString(LabelText));
        Button->AddChild(Label);

        Button->OnClicked.AddDynamic(Button, &UCoreCommandButton::HandleClicked);
        CoreListContainer->AddChildToVerticalBox(Button);
    }
}

void UCommandMenu::OnSkillChosen(FName SkillID)
{
    if (BoundController)
    {
        BoundController->SelectSkill(SkillID);
    }
}

void UCommandMenu::OnTargetChosen(AActor* Target)
{
    if (BoundController)
    {
        BoundController->SelectTarget(Target);
        BoundController->ConfirmAction();
    }
}

void UCommandMenu::OnCoreChosen(FName CoreID)
{
    if (BoundController)
    {
        // Rebuilds this whole menu (skills + cores) via ShowCommandMenu -> BeginSelection.
        BoundController->RequestChangeCore(CoreID);
    }
}
