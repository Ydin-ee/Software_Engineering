#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageFloatingText.generated.h"

class UTextBlock;

// Grayboxing combat feedback: shows a number (or "MISS") and self-destructs after a beat.
// Assign a UTextBlock named "DamageText" in the WBP subclass to see it appear.
UCLASS()
class UDamageFloatingText : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* DamageText;

    UFUNCTION(BlueprintCallable, Category = "RPG|UI")
    void SetDamageInfo(int32 DamageAmount, bool bMissed);

    UPROPERTY(EditDefaultsOnly, Category = "RPG|UI")
    float LifetimeSeconds = 1.0f;

protected:
    virtual void NativeConstruct() override;

private:
    void HandleLifetimeExpired();

    FTimerHandle LifetimeTimerHandle;
};
