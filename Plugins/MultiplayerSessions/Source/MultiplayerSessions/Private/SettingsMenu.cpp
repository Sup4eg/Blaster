// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Button.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "BaseSettingsTab.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Menu.h"
#include "SettingsMenu.h"

void USettingsMenu::NativeConstruct()
{
    Super::NativeConstruct();

    if (GeneralButton && GraphycsButton && ApplyButton && BackButton)
    {
        GeneralButton->OnClicked.AddDynamic(this, &ThisClass::GeneralButtonClicked);
        GraphycsButton->OnClicked.AddDynamic(this, &ThisClass::GraphycsButtonClicked);
        ApplyButton->OnClicked.AddDynamic(this, &ThisClass::ApplyButtonClicked);
        BackButton->OnClicked.AddDynamic(this, &ThisClass::BackButtonClicked);
    }

    // Click General button when create this widget
    if (GeneralButton && GeneralButton->OnClicked.IsBound())
    {
        GeneralButton->OnClicked.Broadcast();
    }
}

void USettingsMenu::GeneralButtonClicked()
{
    if (!GeneralSettingsTabClass) return;
    if (GeneralButton)
    {
        if (GraphycsSettingsTab)
        {
            // Disable previous widget first
            GraphycsSettingsTab->RemoveFromParent();
            if (GraphycsButton)
            {
                GraphycsButton->SetIsEnabled(true);
            }
        }

        GeneralButton->SetIsEnabled(false);
        if (!GeneralSettingsTab && GetWorld())
        {
            GeneralSettingsTab = CreateWidget<UBaseSettingsTab>(GetWorld(), GeneralSettingsTabClass);
        }
        if (GeneralSettingsTab)
        {
            GeneralSettingsTab->AddToViewport();
        }
    }
}

void USettingsMenu::GraphycsButtonClicked()
{
    if (!GraphycsSettingsTabClass) return;
    if (GraphycsButton)
    {
        // Disable previous widget first
        if (GeneralSettingsTab)
        {
            GeneralSettingsTab->RemoveFromParent();
            if (GeneralButton)
            {
                GeneralButton->SetIsEnabled(true);
            }
        }

        GraphycsButton->SetIsEnabled(false);
        if (!GraphycsSettingsTab && GetWorld())
        {
            GraphycsSettingsTab = CreateWidget<UBaseSettingsTab>(GetWorld(), GraphycsSettingsTabClass);
        }
        if (GraphycsSettingsTab)
        {
            GraphycsSettingsTab->AddToViewport();
        }
    }
}

void USettingsMenu::ApplyButtonClicked()
{
    if (GEngine)
    {
        if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
        {
            if (GeneralSettingsTab)
            {
                UserSettings->SetFullscreenMode(GeneralSettingsTab->WindowMode);
                UserSettings->SetVSyncEnabled(GeneralSettingsTab->bIsVsync);
                UserSettings->SetScreenResolution(GeneralSettingsTab->Resolution);
            }
            if (GraphycsSettingsTab)
            {
                UserSettings->SetOverallScalabilityLevel(GraphycsSettingsTab->OverallGraphycsIndex);
                UserSettings->SetViewDistanceQuality(GraphycsSettingsTab->ViewDistanceIndex);
                UserSettings->SetPostProcessingQuality(GraphycsSettingsTab->PostProcessingIndex);
                UserSettings->SetGlobalIlluminationQuality(GraphycsSettingsTab->GlobalIlluminationIndex);
                UserSettings->SetTextureQuality(GraphycsSettingsTab->TexturesIndex);
                UserSettings->SetVisualEffectQuality(GraphycsSettingsTab->VisualEffectsIndex);

                if (GraphycsSettingsTab->OverallGraphycsIndex < 2)
                {
                    UserSettings->SetShadowQuality(2);
                }
            }
            UserSettings->ApplySettings(true);
            UserSettings->LoadSettings(true);
        }
    }
}

void USettingsMenu::BackButtonClicked()
{
    if (!GetWorld()) return;

    if (GeneralSettingsTab)
    {
        GeneralSettingsTab->RemoveFromParent();
    }
    if (GraphycsSettingsTab)
    {
        GraphycsSettingsTab->RemoveFromParent();
    }
    RemoveFromParent();

    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UMenu::StaticClass(), true);
    if (FoundWidgets.Num() > 0)
    {
        if (UMenu* Menu = Cast<UMenu>(FoundWidgets[0]))
        {
            Menu->SetVisibility(ESlateVisibility::Visible);
        }
    }
}
