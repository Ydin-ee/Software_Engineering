#include "RPGMonsterBase.h"
#include "RPGStatComponent.h"
#include "RPGSkillComponent.h"
#include "RPGDataManager.h"
#include "MonsterAIController.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ARPGMonsterBase::ARPGMonsterBase()
{
    // Tick is already disabled in the parent class (ARPGCharacterBase)
    MonsterID = TEXT("MOB_004"); // Default test ID

    AIControllerClass = AMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ARPGMonsterBase::BeginPlay()
{
    Super::BeginPlay();

    // Load stat/skill data when needed
    InitializeMonsterStat();
}

void ARPGMonsterBase::InitializeMonsterStat()
{
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (URPGDataManager* DataManager = GI->GetSubsystem<URPGDataManager>())
        {
            FMonsterDataRow LoadedData;
            if (DataManager->GetMonsterData(MonsterID, LoadedData))
            {
                StatComponent->InitializeStat(LoadedData);

                // Parse the comma-separated skill ID list from the data table
                // into the skill component so the monster can execute them in battle.
                if (SkillComponent)
                {
                    TArray<FString> SkillIDStrings;
                    LoadedData.MonsterSkills.ParseIntoArray(SkillIDStrings, TEXT(","), true);

                    SkillComponent->AvailableSkills.Empty();
                    for (const FString& SkillIDString : SkillIDStrings)
                    {
                        SkillComponent->AvailableSkills.Add(FName(*SkillIDString.TrimStartAndEnd()));
                    }
                }

                FString MName = LoadedData.MonsterName.ToString();
                FString MWeak = LoadedData.Weak;

                UE_LOG(LogTemp, Warning, TEXT("Load Success! Name: %s, Start HP: %d, Weak: %s"),
                    *MName,
                    StatComponent->CurrentHP,
                    *MWeak);
            }
            else
            {
                FString FailID = MonsterID.ToString();
                UE_LOG(LogTemp, Error, TEXT("Load Failed: ID %s not found."), *FailID);
            }
        }
    }
}

void ARPGMonsterBase::AddAggro(AActor* Source, int32 Amount)
{
    if (!Source || Amount <= 0)
    {
        return;
    }

    int32& Aggro = AggroTable.FindOrAdd(Source);
    Aggro += Amount;
}
