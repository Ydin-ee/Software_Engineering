#pragma once

#include "CoreMinimal.h"
#include "MyProjectCharacter.h"
#include "RPGCharacterBase.generated.h"

class URPGStatComponent;
class URPGSkillComponent;

// Inherits AMyProjectCharacter for its camera boom/follow camera and Enhanced Input
// move/look/jump plumbing, shared by both the player and monsters (movement input
// only ever fires while actually possessed by a PlayerController).
UCLASS()
class ARPGCharacterBase : public AMyProjectCharacter
{
    GENERATED_BODY()

public:
    ARPGCharacterBase();

public:
    // ���� �����͸� ����ϴ� ������Ʈ (�÷��̾�/���� ����)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Component")
    URPGStatComponent* StatComponent;

    // ��ų ����� ����ϴ� ������Ʈ (�÷��̾�/���� ����)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Component")
    URPGSkillComponent* SkillComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Stat")
    bool bIsDead;

    // AActor::TakeDamage�� �̸��� ��ġ�� �ʵ��� RPG ���� �Լ��� �и�
    UFUNCTION(BlueprintCallable, Category = "RPG|Combat")
    void TakeRPGDamage(int32 DamageAmount, FName DamageType);
};