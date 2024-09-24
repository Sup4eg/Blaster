#include "BlasterPlayerController.h"
// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterHUD.h"
#include "CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.h"
#include "ElimmedWidget.h"
#include "AnnouncementWidget.h"
#include "BlasterCharacter.h"
#include "BlasterUtils.h"
#include "Engine/World.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterGameMode.h"
#include "CombatComponent.h"
#include "BlasterCharacter.h"
#include "BlasterGameState.h"
#include "BlasterPlayerController.h"

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();
    BlasterHUD = Cast<ABlasterHUD>(GetHUD());
    ServerCheckMatchState();
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SetHUDTime();
    CheckTimeSync(DeltaTime);
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::OnRep_Pawn()
{
    Super::OnRep_Pawn();
    HideHUDElimmed();
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetCharacter());
    if (BlasterCharacter)

    {
        SetLogicDependsOnMatchState(BlasterCharacter);
    }
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
    if (BlasterCharacter)
    {
        SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
        HideHUDElimmed();
        SetLogicDependsOnMatchState(BlasterCharacter);
    }
}

void ABlasterPlayerController::SetLogicDependsOnMatchState(ABlasterCharacter* BlasterCharacter)
{
    if (MatchState == MatchState::Cooldown)
    {
        BlasterCharacter->SetIsGameplayDisabled(true);
        BlasterCharacter->SetUpInputMappingContext(CooldownMappingContext);
    }
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
    bool bHUDValid = IsCharacterOverlayValid() &&                //
                     BlasterHUD->CharacterOverlay->HealthBar &&  //
                     BlasterHUD->CharacterOverlay->HealthText;
    if (bHUDValid)
    {
        const float HealthPercent = Health / MaxHealth;
        const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
        BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
        BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
    }
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
    bool bHUDValid = IsCharacterOverlayValid() && BlasterHUD->CharacterOverlay->ScoreAmount;
    if (bHUDValid)
    {
        FString ScoreText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Score));
        BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
    }
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
    bool bHUDValid = IsCharacterOverlayValid() && BlasterHUD->CharacterOverlay->DefeatsAmount;
    if (bHUDValid)
    {
        FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
        BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
    }
}

void ABlasterPlayerController::ShowHUDElimmed(const FName& KilledBy)
{
    bool bHUDValid = IsCharacterOverlayValid() &&                    //
                     BlasterHUD->CharacterOverlay->ElimmedWidget &&  //
                     BlasterHUD->CharacterOverlay->ElimmedWidget->KilledBy;

    if (bHUDValid)
    {
        BlasterHUD->CharacterOverlay->ElimmedWidget->KilledBy->SetText(FText::FromName(KilledBy));
        BlasterHUD->CharacterOverlay->ElimmedWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void ABlasterPlayerController::HideHUDElimmed()
{
    bool bHUDValid = IsCharacterOverlayValid() &&                    //
                     BlasterHUD->CharacterOverlay->ElimmedWidget &&  //
                     BlasterHUD->CharacterOverlay->ElimmedWidget->KilledBy;

    if (bHUDValid)
    {
        BlasterHUD->CharacterOverlay->ElimmedWidget->KilledBy->SetText(FText());
        BlasterHUD->CharacterOverlay->ElimmedWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
    bool bHUDValid = IsCharacterOverlayValid() &&                     //
                     BlasterHUD->CharacterOverlay->WeaponAmmoAmount;  //

    if (bHUDValid)
    {
        FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
        BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
    }
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
    bool bHUDValid = IsCharacterOverlayValid() &&                      //
                     BlasterHUD->CharacterOverlay->CarriedAmmoAmount;  //

    if (bHUDValid)
    {
        FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
        BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
    }
}

void ABlasterPlayerController::SetHUDWeaponIcon(UTexture2D* WeaponIcon)
{

    bool bHUDValid = IsCharacterOverlayValid() &&              //
                     BlasterHUD->CharacterOverlay->WeaponImg;  //
    if (bHUDValid && WeaponIcon)
    {
        BlasterHUD->CharacterOverlay->WeaponImg->SetBrushFromTexture(WeaponIcon);
    }
}

void ABlasterPlayerController::HideHUDWeaponAmmoBox()
{
    bool bHUDValid = IsCharacterOverlayValid() &&              //
                     BlasterHUD->CharacterOverlay->WeaponBox;  //
    if (bHUDValid)
    {
        BlasterHUD->CharacterOverlay->WeaponBox->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void ABlasterPlayerController::ShowHUDWeaponAmmoBox()
{
    bool bHUDValid = IsCharacterOverlayValid() &&              //
                     BlasterHUD->CharacterOverlay->WeaponBox;  //
    if (bHUDValid)
    {
        BlasterHUD->CharacterOverlay->WeaponBox->SetVisibility(ESlateVisibility::Visible);
    }
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{

    bool bHUDValid = IsCharacterOverlayValid() &&                       //
                     BlasterHUD->CharacterOverlay->MatchCountdownText;  //
    if (bHUDValid)
    {
        SetHUDCountdown(CountdownTime, BlasterHUD->CharacterOverlay->MatchCountdownText);
        BlasterHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FColor::White);
        float MatchTimeLeftAlert = MatchTime * MatchTimeLeftAlertPercentage;
        if (CountdownTime <= MatchTimeLeftAlert)
        {
            BlasterHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FColor::Red);
            BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->CountdownAnimation);
        }
    }
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
    bool bHUDValid = IsAnnouncementWidgetValid() && BlasterHUD->AnnouncementWidget->WarmupTime;
    if (bHUDValid)
    {

        SetHUDCountdown(CountdownTime, BlasterHUD->AnnouncementWidget->WarmupTime);
    }
}

void ABlasterPlayerController::SetHUDCountdown(float CountdownTime, UTextBlock* TimeTextBlock)
{
    if (CountdownTime < 0.f)
    {
        TimeTextBlock->SetText(FText::FromString(FString("00:00")));
        return;
    }

    int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
    int32 Seconds = CountdownTime - Minutes * 60;

    FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    TimeTextBlock->SetText(FText::FromString(CountdownText));
}

void ABlasterPlayerController::SetHUDTime()
{
    if (!GetWorld()) return;

    float TimeLeft = GetTimeLeft();
    uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

    if (CountdownInt != SecondsLeft)
    {
        if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
        {
            SetHUDAnnouncementCountdown(TimeLeft);
        }
        if (MatchState == MatchState::InProgress)
        {
            SetHUDMatchCountdown(TimeLeft);
        }
    }
    CountdownInt = SecondsLeft;
}

float ABlasterPlayerController::GetTimeLeft()
{
    float TimeLeft = 0.f;

    if (HasAuthority() && IsGameModeValid())
    {
        TimeLeft = BlasterGameMode->GetCountdownTime();
    }
    else
    {
        if (MatchState == MatchState::WaitingToStart)
        {
            TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
        }
        else if (MatchState == MatchState::InProgress)
        {
            TimeLeft = MatchTime + WarmupTime - GetServerTime() + LevelStartingTime;
        }
        else if (MatchState == MatchState::Cooldown)
        {
            TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
        }
    }

    return TimeLeft;
}

uint32 ABlasterPlayerController::GetSecondsLeft(float TimeLeft)
{
    uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

    if (HasAuthority() && IsGameModeValid())
    {
        SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + BlasterGameMode->LevelStartingTime);
    }
    return SecondsLeft;
}

void ABlasterPlayerController::ServerRequeestServerTime_Implementation(float TimeOfClientRequest)
{
    if (!GetWorld()) return;
    float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
    ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
    if (!GetWorld()) return;
    float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
    float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime / 2);
    ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
    TimeSyncRunningTime += DeltaTime;
    if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
    {
        ServerRequeestServerTime(GetWorld()->GetTimeSeconds());
        TimeSyncRunningTime = 0.f;
    }
}

float ABlasterPlayerController::GetServerTime()
{
    if (!GetWorld()) return 0.f;
    if (HasAuthority())
    {
        return GetWorld()->GetTimeSeconds();
    }
    else
    {
        return GetWorld()->GetTimeSeconds() + ClientServerDelta;
    }
}

void ABlasterPlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();
    if (IsLocalController())
    {
        ServerRequeestServerTime(GetWorld()->GetTimeSeconds());
    }
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
    MatchState = State;
    HandleMatchState();
}

void ABlasterPlayerController::OnRep_MatchState()
{
    HandleMatchState();
}

void ABlasterPlayerController::HandleMatchState()
{
    if (MatchState == MatchState::InProgress)
    {
        HandleMatchHasStarted();
    }
    else if (MatchState == MatchState::Cooldown)
    {
        HandleMatchCooldown();
    }
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
    if (IsCharacterOverlayValid())
    {
        BlasterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Visible);
    }
    if (IsAnnouncementWidgetValid())
    {
        BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void ABlasterPlayerController::HandleMatchCooldown()
{
    if (IsCharacterOverlayValid())
    {
        BlasterHUD->CharacterOverlay->RemoveFromParent();
    }
    if (IsAnnouncementWidgetValid() && BlasterHUD->AnnouncementWidget->AnnouncementText && BlasterHUD->AnnouncementWidget->InfoText)
    {
        ShowHUDAnnouncement();
    }
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetCharacter());
    if (BlasterCharacter)
    {
        BlasterCharacter->SetIsGameplayDisabled(true);
        BlasterCharacter->SetUpInputMappingContext(CooldownMappingContext);
        if (BlasterCharacter->GetCombatComponent())
        {
            BlasterCharacter->GetCombatComponent()->SetAiming(false);
            BlasterCharacter->GetCombatComponent()->FireButtonPressed(false);
        }
    }
}

void ABlasterPlayerController::ShowHUDAnnouncement()
{
    FString AnnouncementText("New Match Starts In:");
    BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnouncementText));

    ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
    ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
    if (BlasterGameState && BlasterPlayerState)
    {
        TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
        FString InfoTextString;
        if (TopPlayers.IsEmpty())
        {
            InfoTextString = FString("There is no winner.");
        }
        else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
        {
            InfoTextString = FString("You are the winner!");
        }
        else if (TopPlayers.Num() == 1)
        {
            InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
        }
        else if (TopPlayers.Num() > 1)
        {
            InfoTextString = FString("Players tied for the win:\n");
            for (auto TiedPlayer : TopPlayers)
            {
                InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
            }
        }
        BlasterHUD->AnnouncementWidget->InfoText->SetText(FText::FromString(InfoTextString));
    }

    BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
    ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GameMode) return;

    WarmupTime = GameMode->WarmupTime;
    MatchTimeLeftAlertPercentage = GameMode->MatchTimeLeftAlertPercentage;
    MatchTime = GameMode->MatchTime;
    LevelStartingTime = GameMode->LevelStartingTime;
    MatchState = GameMode->GetMatchState();
    CooldownTime = GameMode->CooldownTime;
    ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime, MatchTimeLeftAlertPercentage);

    if (BlasterHUD && MatchState == MatchState::WaitingToStart)
    {
        BlasterHUD->AddAnnouncementWidget();
    }
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch,  //
    float Warmup,                                                                    //
    float Match,                                                                     //
    float StartingTime,                                                              //
    float TimeOfCooldown,                                                            //
    float TimeOfMatchLeftAlertPercentage)
{
    WarmupTime = Warmup;
    MatchTime = Match;
    LevelStartingTime = StartingTime;
    MatchState = StateOfMatch;
    CooldownTime = TimeOfCooldown;
    MatchTimeLeftAlertPercentage = TimeOfMatchLeftAlertPercentage;
    OnMatchStateSet(MatchState);

    if (BlasterHUD && MatchState == MatchState::WaitingToStart)
    {
        BlasterHUD->AddAnnouncementWidget();
    }
}

bool ABlasterPlayerController::IsHUDValid()
{
    return BlasterUtils::CastOrUseExistsActor<ABlasterHUD>(BlasterHUD, GetHUD());
}

bool ABlasterPlayerController::IsCharacterOverlayValid()
{
    return IsHUDValid() && BlasterHUD->CharacterOverlay;
}

bool ABlasterPlayerController::IsAnnouncementWidgetValid()
{
    return IsHUDValid() && BlasterHUD->AnnouncementWidget;
}

bool ABlasterPlayerController::IsGameModeValid()
{

    return BlasterUtils::CastOrUseExistsActor(BlasterGameMode, UGameplayStatics::GetGameMode(this));
}
