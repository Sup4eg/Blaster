// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "BlasterCharacter.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Casing.h"
#include "BlasterPlayerController.h"
#include "BlasterUtils.h"
#include "Weapon.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SetReplicateMovement(true);

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
    SetRootComponent(WeaponMesh);
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area Sphere"));
    AreaSphere->SetupAttachment(RootComponent);
    AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
    PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (PickupWidget)
    {
        PickupWidget->SetVisibility(false);
    }

    if (HasAuthority())
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnsphereOverlap);
        AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
    }
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWeapon, WeaponState);
    DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
    if (!PickupWidget) return;
    PickupWidget->SetVisibility(bShowWidget);
}

void AWeapon::Fire(const FVector& HitTarget)
{
    if (!FireAnimation) return;
    WeaponMesh->PlayAnimation(FireAnimation, false);
    const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
    if (AmmoEjectSocket && GetWorld() && CasingClass)
    {
        FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
        GetWorld()->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
    }
    SpendRound();
}

void AWeapon::OnsphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
    if (!BlasterCharacter) return;
    BlasterCharacter->SetOverlappingWeapon(this);
}

void AWeapon::OnSphereEndOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
    if (!BlasterCharacter) return;
    BlasterCharacter->SetOverlappingWeapon(nullptr);
}

void AWeapon::SpendRound()
{
    if (HasAuthority())
    {
        Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
        SetHUDAmmo();
    }
}

void AWeapon::OnRep_Ammo()
{
    SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
    Super::OnRep_Owner();
    if (!GetOwner())
    {
        BlasterOwnerCharacter = nullptr;
        BlasterOwnerController = nullptr;
    }
    else
    {
        SetHUDAmmo();
    }
}

void AWeapon::SetHUDAmmo()
{
    if (BlasterUtils::CastOrUseExistsActors<ABlasterCharacter, ABlasterPlayerController>(
            BlasterOwnerCharacter, BlasterOwnerController, GetOwner()))
    {
        BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
    }
}

void AWeapon::OnRep_WeaponState()
{
    switch (WeaponState)
    {
        case EWeaponState::EWS_Equiped:
            ShowPickupWidget(false);
            WeaponMesh->SetSimulatePhysics(false);
            WeaponMesh->SetEnableGravity(false);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EWeaponState::EWS_Dropped:
            WeaponMesh->SetSimulatePhysics(true);
            WeaponMesh->SetEnableGravity(true);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        default: break;
    }
}

void AWeapon::SetWeaponState(EWeaponState State)
{
    WeaponState = State;
    switch (WeaponState)
    {
        case EWeaponState::EWS_Equiped:
            ShowPickupWidget(false);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            WeaponMesh->SetSimulatePhysics(false);
            WeaponMesh->SetEnableGravity(false);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EWeaponState::EWS_Dropped:
            if (HasAuthority())
            {
                AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
            WeaponMesh->SetSimulatePhysics(true);
            WeaponMesh->SetEnableGravity(true);
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        default: break;
    }
}

bool AWeapon::IsEmpty()
{
    return Ammo <= 0;
}

void AWeapon::Dropped()
{
    SetWeaponState(EWeaponState::EWS_Dropped);
    FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
    WeaponMesh->DetachFromComponent(DetachRules);
    SetOwner(nullptr);
    BlasterOwnerCharacter = nullptr;
    BlasterOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
    Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
    SetHUDAmmo();
}
