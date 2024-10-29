// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

class ABlasterPlayerState;
class ABlasterGameState;

UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
    GENERATED_BODY()

public:
    ATeamsGameMode();

    virtual void PlayerElimmed(                      //
        ABlasterCharacter* ElimmedCharacter,         //
        ABlasterPlayerController* VictimController,  //
        ABlasterPlayerController* AttackerController) override;

    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual void Logout(AController* Exiting) override;

    virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;

protected:
    virtual void HandleMatchHasStarted() override;

private:
    void SortPlayerToTeam(ABlasterPlayerState* BlasterPlayerState, ABlasterGameState* BlasterGameState);
};
