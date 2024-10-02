// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"
#include "CombatComponent.h"
#include "AmmoPickup.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,  //
    AActor* OtherActor,                                                      //
    UPrimitiveComponent* OtherComp,                                          //
    int32 OtherBodyIndex,                                                    //
    bool bFromSweep,                                                         //
    const FHitResult& SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    if (OtherActor->ActorHasTag("BlasterCharacter"))
    {
        if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
        {
            if (UCombatComponent* CombatComp = BlasterCharacter->GetCombatComponent())
            {
                CombatComp->PickupAmmo(WeaponType, AmmoAmount);
            }
        }
    }
    Destroy();
}
