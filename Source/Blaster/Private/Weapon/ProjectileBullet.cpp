// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BlasterCharacter.h"
#include "BlasterPlayerController.h"
#include "LagCompensationComponent.h"
#include "Weapon.h"
#include "ProjectileBullet.h"

AProjectileBullet::AProjectileBullet()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
    ProjectileMovementComponent->InitialSpeed = InitialSpeed;
    ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
    Super::PostEditChangeProperty(Event);

    FName PropertyName = Event.Property ? Event.Property->GetFName() : NAME_None;
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed) && ProjectileMovementComponent)
    {
        ProjectileMovementComponent->InitialSpeed = InitialSpeed;
        ProjectileMovementComponent->MaxSpeed = InitialSpeed;
    }
}
#endif

void AProjectileBullet::OnHit(
    UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()))
    {
        if (ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->GetController()))
        {
            if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
            {
                UGameplayStatics::ApplyDamage(
                    OtherActor, Damage, OwnerController, OwnerCharacter->GetEquippedWeapon(), UDamageType::StaticClass());
                Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
                return;
            }

            if (OtherActor && OtherActor->ActorHasTag("BlasterCharacter"))
            {
                if (ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor))
                {
                    // TODO, Potantial LEAK HERE, Damage and damage causer must be validated
                    if (bUseServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
                    {
                        float HitTime = OwnerController->GetServerTime() - OwnerController->SingleTripTime;
                        OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(  //
                            HitCharacter,                                                             //
                            TraceStart,                                                               //
                            InitialVelocity,                                                          //
                            HitTime,                                                                  //
                            Damage,                                                                   //
                            OwnerCharacter->GetEquippedWeapon());
                    }
                }
            }
        }
    }
    Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
