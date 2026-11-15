// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_PauseMenuWidget.h"
#include "Components/Button.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UCAP_PauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UCAP_PauseMenuWidget::OnResumeClicked);
	}
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UCAP_PauseMenuWidget::OnRestartClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UCAP_PauseMenuWidget::OnQuitClicked);
	}

	SetupActiveButtons();

	for (int32 i = 0; i < ActiveButtons.Num(); i++)
	{
		if (ActiveButtons[i])
		{
			ActiveButtons[i]->OnHovered.AddDynamic(this, &UCAP_PauseMenuWidget::OnBtnHovered);
		}
	}
}

void UCAP_PauseMenuWidget::NativeOpenMenu()
{
	UpdateSelection(0);
}

void UCAP_PauseMenuWidget::NativeCloseMenu()
{
	if (OnMenuClosedDelegate.IsBound())
	{
		OnMenuClosedDelegate.Broadcast();
	}
}

void UCAP_PauseMenuWidget::HandleChangeSelectedSlot(FVector2D InputVal)
{
	if (ActiveButtons.Num() <= 1) return;

	if (InputVal.Y > 0.f)
	{
		int32 NextIndex = (CurrentSelectedIndex - 1 + ActiveButtons.Num()) % ActiveButtons.Num();
		UpdateSelection(NextIndex);
	}
	else if (InputVal.Y < 0.f)
	{
		int32 NextIndex = (CurrentSelectedIndex + 1) % ActiveButtons.Num();
		UpdateSelection(NextIndex);
	}
}

void UCAP_PauseMenuWidget::HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (TriggerEvent != ETriggerEvent::Started) return;

	if (ActiveButtons.IsValidIndex(CurrentSelectedIndex) && ActiveButtons[CurrentSelectedIndex])
	{
		ActiveButtons[CurrentSelectedIndex]->OnClicked.Broadcast();
	}
}

void UCAP_PauseMenuWidget::SetupActiveButtons()
{
	ActiveButtons.Empty();
	ActiveTextBlocks.Empty();

	auto RegisterButton = [&](UButton* InBtn, UTextBlock* InText)
	{
		if (InBtn && InText)
		{
			ActiveButtons.Add(InBtn);
			ActiveTextBlocks.Add(InText);
		}
	};

	RegisterButton(ResumeButton, ResumeText);
	RegisterButton(RestartButton, RestartText);
	RegisterButton(QuitButton, QuitText);

	CurrentSelectedIndex = 0;
	RefreshButtonVisuals();
}

void UCAP_PauseMenuWidget::RefreshButtonVisuals()
{
	for (int32 i = 0; i < ActiveButtons.Num(); i++)
	{
		if (!ActiveButtons[i] || !ActiveTextBlocks[i]) continue;

		bool bIsFocused = (CurrentSelectedIndex == i);
		UCAP_WidgetHelper::ApplyCustomButtonStyle(ActiveButtons[i], ActiveTextBlocks[i], bIsFocused, ButtonSettings);
	}
}

void UCAP_PauseMenuWidget::OnBtnHovered()
{
	for (int32 i = 0; i < ActiveButtons.Num(); i++)
	{
		if (ActiveButtons[i] && ActiveButtons[i]->IsHovered())
		{
			CurrentSelectedIndex = i;
			RefreshButtonVisuals();
			break;
		}
	}
}

void UCAP_PauseMenuWidget::UpdateSelection(int32 NewIndex)
{
	if (ActiveButtons.IsEmpty()) return;

	CurrentSelectedIndex = FMath::Clamp(NewIndex, 0, ActiveButtons.Num() - 1);
	RefreshButtonVisuals();
}

void UCAP_PauseMenuWidget::OnResumeClicked()
{
	NativeCloseMenu();
}

void UCAP_PauseMenuWidget::OnRestartClicked()
{
	// 현재 런 동안 누적된 무기, 아이템, 재화 및 통계 데이터 즉시 초기화
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UCAP_ProgressionSubsystem* Subsys = GI->GetSubsystem<UCAP_ProgressionSubsystem>())
		{
			Subsys->ClearRunStats();
		}
	}

	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, VillageLevelName);
}

void UCAP_PauseMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}