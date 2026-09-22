#include "CommandMenu.h"
#include "RPGPlayerController.h"
#include "PlayerCharacter.h"
#include "RPGSkillComponent.h"
#include "RPGDataManager.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

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

        USkillCommandButton* Button = NewObject<USkillCommandButton>(this);
        Button->SkillID = SkillID;
        Button->OwningMenu = this;
        Button->SetIsEnabled(bUnlocked);

        UTextBlock* Label = NewObject<UTextBlock>(this);
        Label->SetText(FText::FromString(bUnlocked ? SkillID.ToString() : SkillID.ToString() + TEXT(" (Locked)")));
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

    for (AActor* Target : PossibleTargets)
    {
        if (!Target)
        {
            continue;
        }

        UTargetCommandButton* Button = NewObject<UTargetCommandButton>(this);
        Button->TargetActor = Target;
        Button->OwningMenu = this;

        UTextBlock* Label = NewObject<UTextBlock>(this);
        Label->SetText(FText::FromString(Target->GetName()));
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

        UTextBlock* Label = NewObject<UTextBlock>(this);
        Label->SetText(FText::FromString(bUnlocked ? CoreID.ToString() : CoreID.ToString() + TEXT(" (Locked)")));
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
