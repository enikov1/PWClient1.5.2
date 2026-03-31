// Filename	: DlgOptimizeUI.cpp
// Creator	: Auto-generated
// Date		: 2024

#include "DlgOptimizeUI.h"
#include "EC_Global.h"
#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EC_Configs.h"
#include "EC_GameUIMan.h"

#include <AUICheckBox.h>
#include <AUISlider.h>

#define new A_DEBUG_NEW

AUI_BEGIN_COMMAND_MAP(CDlgOptimizeUI, CDlgOptimizeBase)
AUI_ON_COMMAND("apply", OnCommand_Apply)
AUI_ON_COMMAND("Btn_Decide", OnCommand_Confirm)
AUI_ON_COMMAND("HelpName_Win_OptimizeGfx", OnCommand_Confirm)
AUI_ON_COMMAND("Btn_UIDefault", OnCommand_UIDefault)
AUI_ON_COMMAND("Slider_Target", OnCommand_SliderTarget)
AUI_ON_COMMAND("Slider_Map", OnCommand_SliderMap)
AUI_ON_COMMAND("Slider_Quickbar", OnCommand_SliderQuickbar)
AUI_ON_COMMAND("Slider_Chat", OnCommand_SliderChat)
AUI_ON_COMMAND("Slider_Team", OnCommand_SliderTeam)
AUI_ON_COMMAND("Slider_Elf", OnCommand_SliderElf)
AUI_ON_COMMAND("Slider_Amulet", OnCommand_SliderAmulet)
AUI_ON_COMMAND("Slider_Inventory", OnCommand_SliderInventory)
AUI_ON_COMMAND("Slider_Option", OnCommand_SliderOption)
AUI_ON_COMMAND("Slider_Other", OnCommand_SliderOther)
AUI_ON_COMMAND("kwi_targethp", OnCommand_kwi_targethp)
AUI_ON_COMMAND("kwi_teamhp", OnCommand_kwi_teamhp)
AUI_ON_COMMAND("kwi_elfmode", OnCommand_kwi_elfmode)
AUI_ON_COMMAND("kwi_title_enable", OnCommand_kwi_title_enable)
AUI_ON_COMMAND("kwi_extrapanelsize", OnCommand_kwi_extrapanelsize)
AUI_ON_COMMAND("kwi_targetcastskill", OnCommand_kwi_targetcastskill)
AUI_ON_COMMAND("kwi_setcustomwnd", OnCommand_kwi_setcustomwnd)
AUI_END_COMMAND_MAP()


CDlgOptimizeUI::CDlgOptimizeUI()
: m_pSliderTarget(NULL)
, m_pSliderMap(NULL)
, m_pSliderQuickbar(NULL)
, m_pSliderChat(NULL)
, m_pSliderTeam(NULL)
, m_pSliderElf(NULL)
, m_pSliderAmulet(NULL)
, m_pSliderInventory(NULL)
, m_pSliderOption(NULL)
, m_pSliderOther(NULL)
, m_pTxtCurTarget(NULL)
, m_pTxtCurMap(NULL)
, m_pTxtCurQuickbar(NULL)
, m_pTxtCurChat(NULL)
, m_pTxtCurTeam(NULL)
, m_pTxtCurElf(NULL)
, m_pTxtCurAmulet(NULL)
, m_pTxtCurInventory(NULL)
, m_pTxtCurOption(NULL)
, m_pTxtCurOther(NULL)
, m_pChkTargetHP(NULL)
, m_pChkTeamHP(NULL)
, m_pChkElfMode(NULL)
, m_pChkHideTitle(NULL)
, m_pChkExtraPanelSize(NULL)
, m_pChkShowTargetCast(NULL)
, m_pChkSetCustomWindowTitle(NULL)
{
}

CDlgOptimizeUI::~CDlgOptimizeUI()
{
}

bool CDlgOptimizeUI::OnInitDialog()
{
	// Sliders
	DDX_Control("Slider_Target", m_pSliderTarget);
	DDX_Control("Slider_Map", m_pSliderMap);
	DDX_Control("Slider_Quickbar", m_pSliderQuickbar);
	DDX_Control("Slider_Chat", m_pSliderChat);
	DDX_Control("Slider_Team", m_pSliderTeam);
	DDX_Control("Slider_Elf", m_pSliderElf);
	DDX_Control("Slider_Amulet", m_pSliderAmulet);
	DDX_Control("Slider_Inventory", m_pSliderInventory);
	DDX_Control("Slider_Option", m_pSliderOption);
	DDX_Control("Slider_Other", m_pSliderOther);
	
	// Labels for current values
	DDX_Control("Txt_CurTarget", m_pTxtCurTarget);
	DDX_Control("Txt_CurMap", m_pTxtCurMap);
	DDX_Control("Txt_CurQuickbar", m_pTxtCurQuickbar);
	DDX_Control("Txt_CurChat", m_pTxtCurChat);
	DDX_Control("Txt_CurTeam", m_pTxtCurTeam);
	DDX_Control("Txt_CurElf", m_pTxtCurElf);
	DDX_Control("Txt_CurAmulet", m_pTxtCurAmulet);
	DDX_Control("Txt_CurInventory", m_pTxtCurInventory);
	DDX_Control("Txt_CurOption", m_pTxtCurOption);
	DDX_Control("Txt_CurOther", m_pTxtCurOther);
	
	// Checkboxes
	DDX_Control("kwi_targethp", m_pChkTargetHP);
	DDX_Control("kwi_teamhp", m_pChkTeamHP);
	DDX_Control("kwi_elfmode", m_pChkElfMode);
	DDX_Control("kwi_title_enable", m_pChkHideTitle);
	DDX_Control("kwi_extrapanelsize", m_pChkExtraPanelSize);
	DDX_Control("kwi_targetcastskill", m_pChkShowTargetCast);
	DDX_Control("kwi_setcustomwnd", m_pChkSetCustomWindowTitle);

	return CDlgOptimizeBase::OnInitDialog();
}

void CDlgOptimizeUI::OnShowDialog()
{
	m_uiSetting = GetGame()->GetConfigs()->GetUISettings();
	UpdateUIView();
	// Apply scales when dialog is shown (to update UI immediately)
	ApplyUIScales();
}

void CDlgOptimizeUI::UpdateUIView()
{
	// Update sliders
	if (m_pSliderTarget) m_pSliderTarget->SetLevel(m_uiSetting.nScaleTarget);
	if (m_pSliderMap) m_pSliderMap->SetLevel(m_uiSetting.nScaleMap);
	if (m_pSliderQuickbar) m_pSliderQuickbar->SetLevel(m_uiSetting.nScaleQuickbar);
	if (m_pSliderChat) m_pSliderChat->SetLevel(m_uiSetting.nScaleChat);
	if (m_pSliderTeam) m_pSliderTeam->SetLevel(m_uiSetting.nScaleTeam);
	if (m_pSliderElf) m_pSliderElf->SetLevel(m_uiSetting.nScaleElf);
	if (m_pSliderAmulet) m_pSliderAmulet->SetLevel(m_uiSetting.nScaleAmulet);
	if (m_pSliderInventory) m_pSliderInventory->SetLevel(m_uiSetting.nScaleInventory);
	if (m_pSliderOption) m_pSliderOption->SetLevel(m_uiSetting.nScaleOption);
	if (m_pSliderOther) m_pSliderOther->SetLevel(m_uiSetting.nScaleOther);
	
	// Update labels
	UpdateScaleLabel(m_pTxtCurTarget, m_uiSetting.nScaleTarget);
	UpdateScaleLabel(m_pTxtCurMap, m_uiSetting.nScaleMap);
	UpdateScaleLabel(m_pTxtCurQuickbar, m_uiSetting.nScaleQuickbar);
	UpdateScaleLabel(m_pTxtCurChat, m_uiSetting.nScaleChat);
	UpdateScaleLabel(m_pTxtCurTeam, m_uiSetting.nScaleTeam);
	UpdateScaleLabel(m_pTxtCurElf, m_uiSetting.nScaleElf);
	UpdateScaleLabel(m_pTxtCurAmulet, m_uiSetting.nScaleAmulet);
	UpdateScaleLabel(m_pTxtCurInventory, m_uiSetting.nScaleInventory);
	UpdateScaleLabel(m_pTxtCurOption, m_uiSetting.nScaleOption);
	UpdateScaleLabel(m_pTxtCurOther, m_uiSetting.nScaleOther);
	
	// Update checkboxes
	if (m_pChkTargetHP) m_pChkTargetHP->Check(m_uiSetting.bShowTargetHP);
	if (m_pChkTeamHP) m_pChkTeamHP->Check(m_uiSetting.bShowTeamHP);
	if (m_pChkElfMode) m_pChkElfMode->Check(m_uiSetting.bElfMode);
	if (m_pChkHideTitle) m_pChkHideTitle->Check(m_uiSetting.bHideTitle);
	if (m_pChkExtraPanelSize) m_pChkExtraPanelSize->Check(m_uiSetting.bExtraPanelSize);
	if (m_pChkShowTargetCast) m_pChkShowTargetCast->Check(m_uiSetting.bShowTargetCast);
	if (m_pChkSetCustomWindowTitle) m_pChkSetCustomWindowTitle->Check(m_uiSetting.bSetCustomWindowTitle);
}

void CDlgOptimizeUI::UpdateScaleLabel(AUIObject* pLabel, BYTE level)
{
	if (!pLabel) return;
	
	float scale = EC_UI_SETTING::LevelToScale(level);
	ACString strText;
	strText.Format(L"%d%%", (int)(scale * 100.0f + 0.5f));
	pLabel->SetText(strText);
}

void CDlgOptimizeUI::OnCommand_Apply(const char *szCommand)
{
	// Save current settings
	if (m_pSliderTarget) m_uiSetting.nScaleTarget = (BYTE)m_pSliderTarget->GetLevel();
	if (m_pSliderMap) m_uiSetting.nScaleMap = (BYTE)m_pSliderMap->GetLevel();
	if (m_pSliderQuickbar) m_uiSetting.nScaleQuickbar = (BYTE)m_pSliderQuickbar->GetLevel();
	if (m_pSliderChat) m_uiSetting.nScaleChat = (BYTE)m_pSliderChat->GetLevel();
	if (m_pSliderTeam) m_uiSetting.nScaleTeam = (BYTE)m_pSliderTeam->GetLevel();
	if (m_pSliderElf) m_uiSetting.nScaleElf = (BYTE)m_pSliderElf->GetLevel();
	if (m_pSliderAmulet) m_uiSetting.nScaleAmulet = (BYTE)m_pSliderAmulet->GetLevel();
	if (m_pSliderInventory) m_uiSetting.nScaleInventory = (BYTE)m_pSliderInventory->GetLevel();
	if (m_pSliderOption) m_uiSetting.nScaleOption = (BYTE)m_pSliderOption->GetLevel();
	if (m_pSliderOther) m_uiSetting.nScaleOther = (BYTE)m_pSliderOther->GetLevel();
	
		if (m_pChkTargetHP) m_uiSetting.bShowTargetHP = m_pChkTargetHP->IsChecked();
		if (m_pChkTeamHP) m_uiSetting.bShowTeamHP = m_pChkTeamHP->IsChecked();
		if (m_pChkElfMode) m_uiSetting.bElfMode = m_pChkElfMode->IsChecked();
		if (m_pChkHideTitle) m_uiSetting.bHideTitle = m_pChkHideTitle->IsChecked();
		if (m_pChkExtraPanelSize) m_uiSetting.bExtraPanelSize = m_pChkExtraPanelSize->IsChecked();
		if (m_pChkShowTargetCast) m_uiSetting.bShowTargetCast = m_pChkShowTargetCast->IsChecked();
		if (m_pChkSetCustomWindowTitle) m_uiSetting.bSetCustomWindowTitle = m_pChkSetCustomWindowTitle->IsChecked();
	
	GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	ApplyUIScales();
	
	// Save settings to server
	if (g_pGame && g_pGame->GetGameRun())
		g_pGame->GetGameRun()->SaveConfigsToServer();
}

void CDlgOptimizeUI::OnCommand_Confirm(const char *szCommand)
{
	OnCommand_Apply(szCommand);
	Show(false);
}

void CDlgOptimizeUI::OnCommand_UIDefault(const char *szCommand)
{
	m_uiSetting.Reset();
	UpdateUIView();
	// Apply default settings immediately (preview)
	GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
}

// Slider command handlers (update immediately for preview, but don't save until Apply)
void CDlgOptimizeUI::OnCommand_SliderTarget(const char *szCommand)
{
	if (m_pSliderTarget)
	{
		BYTE level = (BYTE)m_pSliderTarget->GetLevel();
		m_uiSetting.nScaleTarget = level;
		UpdateScaleLabel(m_pTxtCurTarget, level);
		// Apply scale immediately for preview (but don't save yet)
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderMap(const char *szCommand)
{
	if (m_pSliderMap)
	{
		BYTE level = (BYTE)m_pSliderMap->GetLevel();
		m_uiSetting.nScaleMap = level;
		UpdateScaleLabel(m_pTxtCurMap, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderQuickbar(const char *szCommand)
{
	if (m_pSliderQuickbar)
	{
		BYTE level = (BYTE)m_pSliderQuickbar->GetLevel();
		m_uiSetting.nScaleQuickbar = level;
		UpdateScaleLabel(m_pTxtCurQuickbar, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderChat(const char *szCommand)
{
	if (m_pSliderChat)
	{
		BYTE level = (BYTE)m_pSliderChat->GetLevel();
		m_uiSetting.nScaleChat = level;
		UpdateScaleLabel(m_pTxtCurChat, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderTeam(const char *szCommand)
{
	if (m_pSliderTeam)
	{
		BYTE level = (BYTE)m_pSliderTeam->GetLevel();
		m_uiSetting.nScaleTeam = level;
		UpdateScaleLabel(m_pTxtCurTeam, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderElf(const char *szCommand)
{
	if (m_pSliderElf)
	{
		BYTE level = (BYTE)m_pSliderElf->GetLevel();
		m_uiSetting.nScaleElf = level;
		UpdateScaleLabel(m_pTxtCurElf, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderAmulet(const char *szCommand)
{
	if (m_pSliderAmulet)
	{
		BYTE level = (BYTE)m_pSliderAmulet->GetLevel();
		m_uiSetting.nScaleAmulet = level;
		UpdateScaleLabel(m_pTxtCurAmulet, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderInventory(const char *szCommand)
{
	if (m_pSliderInventory)
	{
		BYTE level = (BYTE)m_pSliderInventory->GetLevel();
		m_uiSetting.nScaleInventory = level;
		UpdateScaleLabel(m_pTxtCurInventory, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderOption(const char *szCommand)
{
	if (m_pSliderOption)
	{
		BYTE level = (BYTE)m_pSliderOption->GetLevel();
		m_uiSetting.nScaleOption = level;
		UpdateScaleLabel(m_pTxtCurOption, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

void CDlgOptimizeUI::OnCommand_SliderOther(const char *szCommand)
{
	if (m_pSliderOther)
	{
		BYTE level = (BYTE)m_pSliderOther->GetLevel();
		m_uiSetting.nScaleOther = level;
		UpdateScaleLabel(m_pTxtCurOther, level);
		// Apply scale immediately for preview
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
	}
}

// Checkbox command handlers (update settings immediately and save)
void CDlgOptimizeUI::OnCommand_kwi_targethp(const char *szCommand)
{
	if (m_pChkTargetHP)
	{
		m_uiSetting.bShowTargetHP = m_pChkTargetHP->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeUI::OnCommand_kwi_teamhp(const char *szCommand)
{
	if (m_pChkTeamHP)
	{
		m_uiSetting.bShowTeamHP = m_pChkTeamHP->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeUI::OnCommand_kwi_elfmode(const char *szCommand)
{
	if (m_pChkElfMode)
	{
		m_uiSetting.bElfMode = m_pChkElfMode->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeUI::OnCommand_kwi_title_enable(const char *szCommand)
{
	if (m_pChkHideTitle)
	{
		m_uiSetting.bHideTitle = m_pChkHideTitle->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

// FPS counter functionality removed - was causing issues

void CDlgOptimizeUI::OnCommand_kwi_extrapanelsize(const char *szCommand)
{
	if (m_pChkExtraPanelSize)
	{
		m_uiSetting.bExtraPanelSize = m_pChkExtraPanelSize->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeUI::OnCommand_kwi_targetcastskill(const char *szCommand)
{
	if (m_pChkShowTargetCast)
	{
		m_uiSetting.bShowTargetCast = m_pChkShowTargetCast->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeUI::OnCommand_kwi_setcustomwnd(const char *szCommand)
{
	if (m_pChkSetCustomWindowTitle)
	{
		m_uiSetting.bSetCustomWindowTitle = m_pChkSetCustomWindowTitle->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(m_uiSetting);
		// Update window title immediately (UpdateWindowTitle() has all necessary checks inside)
		GetGame()->GetConfigs()->UpdateWindowTitle();
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeUI::ApplyUIScales()
{
	CECGameUIMan* pUIMan = GetGameUIMan();
	if (!pUIMan) return;

	PAUIDIALOG pDlg;
	
	// Apply target scale to all target dialogs
	float fTargetScale = EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleTarget);
	const char* szTargetDialogs[] = {
		"Win_HpOtherB",
		"Win_HpOther",
		"Win_Hpmpxp"
	};
	for (int i = 0; i < sizeof(szTargetDialogs) / sizeof(szTargetDialogs[0]); i++)
	{
		pDlg = pUIMan->GetDialog(szTargetDialogs[i]);
		if (pDlg)
			pDlg->SetScale(fTargetScale);
	}
	
	pDlg = pUIMan->GetDialog("Win_Map");
	if (pDlg)
		pDlg->SetScale(EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleMap));
	
	// Apply quickbar scale to all quickbar dialogs
	float fQuickbarScale = EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleQuickbar);
	const char* szQuickbarDialogs[] = {
		"Win_Quickbar13Va",
		"Win_Quickbar13Vb_1", "Win_Quickbar13Vb_2", "Win_Quickbar13Vb_3", "Win_Quickbar13Vb_4", "Win_Quickbar13Vb_5",
		"Win_Quickbar12Ha", "Win_Quickbar12Hb_1", "Win_Quickbar12Hb_2", "Win_Quickbar12Hb_3",
		"Win_Quickbar12Va", "Win_Quickbar12Vb_1", "Win_Quickbar12Vb_2", "Win_Quickbar12Vb_3",
		"Win_Quickbar13Ha", "Win_Quickbar13Hb_1", "Win_Quickbar13Hb_2", "Win_Quickbar13Hb_3", "Win_Quickbar13Hb_4", "Win_Quickbar13Hb_5"
	};
	for (int i = 0; i < sizeof(szQuickbarDialogs) / sizeof(szQuickbarDialogs[0]); i++)
	{
		pDlg = pUIMan->GetDialog(szQuickbarDialogs[i]);
		if (pDlg)
			pDlg->SetScale(fQuickbarScale);
	}
	
	// Apply chat scale to all chat dialogs
	float fChatScale = EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleChat);
	pDlg = pUIMan->GetDialog("Win_Chat");
	if (pDlg)
		pDlg->SetScale(fChatScale);
	pDlg = pUIMan->GetDialog("Win_ChatSmall");
	if (pDlg)
		pDlg->SetScale(fChatScale);
	
	// Apply team scale to all team dialogs
	float fTeamScale = EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleTeam);
	const char* szTeamDialogs[] = {
		"Win_TeamMain",
		"Win_TeamMate1", "Win_TeamMate2", "Win_TeamMate3", "Win_TeamMate4", "Win_TeamMate5",
		"Win_TeamMate6", "Win_TeamMate7", "Win_TeamMate8", "Win_TeamMate9"
	};
	for (int i = 0; i < sizeof(szTeamDialogs) / sizeof(szTeamDialogs[0]); i++)
	{
		pDlg = pUIMan->GetDialog(szTeamDialogs[i]);
		if (pDlg)
			pDlg->SetScale(fTeamScale);
	}
	
	pDlg = pUIMan->GetDialog("Win_ELF");
	if (pDlg)
		pDlg->SetScale(EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleElf));
	
	// Apply amulet scale to all amulet dialogs
	float fAmuletScale = EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleAmulet);
	const char* szAmuletDialogs[] = {
		"Win_Amulet",
		"Win_AmuletMain",
		"Win_PrgsHP",
		"Win_PrgsMP"
	};
	for (int i = 0; i < sizeof(szAmuletDialogs) / sizeof(szAmuletDialogs[0]); i++)
	{
		pDlg = pUIMan->GetDialog(szAmuletDialogs[i]);
		if (pDlg)
			pDlg->SetScale(fAmuletScale);
	}
	
	pDlg = pUIMan->GetDialog("Win_Inventory");
	if (pDlg)
		pDlg->SetScale(EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleInventory));
	
	// Try common option/setting dialog names
	pDlg = pUIMan->GetDialog("Win_Option");
	if (!pDlg) pDlg = pUIMan->GetDialog("Win_SettingSystem");
	if (!pDlg) pDlg = pUIMan->GetDialog("Win_SettingGame");
	if (pDlg)
		pDlg->SetScale(EC_UI_SETTING::LevelToScale(m_uiSetting.nScaleOption));
	
	// Apply "Other" scale to all other dialogs (optional, can be skipped for now)
	// This would require iterating through all dialogs which might be too heavy
}

