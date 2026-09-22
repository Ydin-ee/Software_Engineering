#include "FieldEncounter.h"
#include "RPGBattleManager.h"
#include "PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AFieldEncounter::AFieldEncounter()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
    TriggerVolume->InitSphereRadius(80.f);
    TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = TriggerVolume;

    // Grayboxing: every field encounter looks like a generic slime regardless of what
    // actually spawns in battle. Assign a mesh on the placed instance/BP.
    SlimeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlimeMesh"));
    SlimeMesh->SetupAttachment(RootComponent);
    SlimeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFieldEncounter::BeginPlay()
{
    Super::BeginPlay();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFieldEncounter::HandleBeginOverlap);
}

void AFieldEncounter::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!Cast<APlayerCharacter>(OtherActor))
    {
        return;
    }

    ARPGBattleManager* BattleManager = Cast<ARPGBattleManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ARPGBattleManager::StaticClass()));
    if (!BattleManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[FieldEncounter] No ARPGBattleManager found in the level."));
        return;
    }

    // Hide/disable immediately so a slow transition or repeated overlap can't retrigger this.
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    BattleManager->BeginFieldEncounter(StageID, BattleID, this);
}
