// Filename	: DlgOptimizeFunc.cpp
// Creator	: zhangyitian
// Date		: 2013/9/27

#include "DlgOptimizeFunc.h"
#include "EC_UIHelper.h"
#include "EC_GameUIMan.h"
#include "DlgIceThunderBall.h"
#include "EC_Global.h"
#include <AUICheckBox.h>


AUI_BEGIN_COMMAND_MAP(CDlgOptimizeFunc, CDlgOptimizeBase)
AUI_ON_COMMAND("Btn_Decide", OnCommand_Confirm)
AUI_ON_COMMAND("Btn_FunSelectAll", OnCommand_SelectAll)
AUI_ON_COMMAND("Btn_FunClear", OnCommand_Clear)
AUI_ON_COMMAND("kwizyyy_radiusname", OnCommand_RadiusName)
AUI_ON_COMMAND("kwizyyy_radiusmodel", OnCommand_RadiusModel)
AUI_ON_COMMAND("kwi_unfreezwnd", OnCommand_Unfreeze)
AUI_ON_COMMAND("kwi_offfly", OnCommand_OffFly)
AUI_ON_COMMAND("kwi_offhair", OnCommand_OffHair)
AUI_ON_COMMAND("kwi_nodelay", OnCommand_NoDelay)
AUI_ON_COMMAND("kwi_autocomplete", OnCommand_AutoComplete)
AUI_ON_COMMAND("kwi_offunselect", OnCommand_OffUnselect)
AUI_ON_COMMAND("kwi_cdtimer", OnCommand_CDTimer)
AUI_ON_COMMAND("kwi_offweapon", OnCommand_OffWeapon)
AUI_ON_COMMAND("kwi_fastfly", OnCommand_FastFly)
AUI_ON_COMMAND("kwi_offbooth", OnCommand_OffBooth)
AUI_ON_COMMAND("kwi_hp", OnCommand_HP)
AUI_ON_COMMAND("kwi_teamhp", OnCommand_TeamHP)
AUI_END_COMMAND_MAP()

CDlgOptimizeFunc::CDlgOptimizeFunc() {
	m_pChkEquipCompare = NULL;
	m_pChkLowHPGfx = NULL;
	m_pChkPetBuff = NULL;
	m_pChkLockQBar = NULL;
	m_pChkHideIceThunderBall = NULL;
	m_pChkHideTargetOfTarget = NULL;
	m_pChkHideElf = NULL;
	m_pChkHideQuest = NULL;
	m_pChkBuff = NULL;
	m_pChkUnfreeze = NULL;
	m_pChkOffFly = NULL;
	m_pChkOffHair = NULL;
	m_pChkNoDelay = NULL;
	m_pChkAutoComplete = NULL;
	m_pChkOffUnselect = NULL;
	m_pChkCDTimer = NULL;
	m_pChkOffWeapon = NULL;
	m_pChkFastFly = NULL;
	m_pChkOffBooth = NULL;
	m_pChkHP = NULL;
	m_pChkTeamHP = NULL;
}

bool CDlgOptimizeFunc::OnInitDialog() {
	DDX_Control("Chk_LowHPGfx", m_pChkLowHPGfx);
	DDX_Control("Chk_EquipCompare", m_pChkEquipCompare);
	DDX_Control("Chk_PetBuff", m_pChkPetBuff);
	DDX_Control("Chk_LockQBar", m_pChkLockQBar);
	DDX_Control("Chk_Ball", m_pChkHideIceThunderBall);
	DDX_Control("Chk_HideTargetOfTarget", m_pChkHideTargetOfTarget);
	DDX_Control("Chk_HideElf", m_pChkHideElf);
	DDX_Control("Chk_HideQuest", m_pChkHideQuest);
	DDX_Control("Chk_Buff", m_pChkBuff);
	DDX_Control("kwi_unfreezwnd", m_pChkUnfreeze);
	DDX_Control("kwi_offfly", m_pChkOffFly);
	DDX_Control("kwi_offhair", m_pChkOffHair);
	DDX_Control("kwi_nodelay", m_pChkNoDelay);
	DDX_Control("kwi_autocomplete", m_pChkAutoComplete);
	DDX_Control("kwi_offunselect", m_pChkOffUnselect);
	DDX_Control("kwi_cdtimer", m_pChkCDTimer);
	DDX_Control("kwi_offweapon", m_pChkOffWeapon);
	DDX_Control("kwi_fastfly", m_pChkFastFly);
	DDX_Control("kwi_offbooth", m_pChkOffBooth);
	DDX_Control("kwi_hp", m_pChkHP);
	DDX_Control("kwi_teamhp", m_pChkTeamHP);
	return CDlgOptimizeBase::OnInitDialog();
}

void CDlgOptimizeFunc::OnShowDialog() {
	m_setting = GetGame()->GetConfigs()->GetGameSettings();
	UpdateFunctionView();
}

void CDlgOptimizeFunc::UpdateFunctionView() {
	CECGameUIMan* pGameUIMan = CECUIHelper::GetGameUIMan();
	m_pChkLowHPGfx->Check(!pGameUIMan->IsShowLowHP());
	m_pChkEquipCompare->Check(!pGameUIMan->IsShowItemDescCompare());
	m_pChkHideTargetOfTarget->Check(!pGameUIMan->IsShowTargetOfTarget());

	m_pChkPetBuff->Check(m_setting.bPetAutoSkill);
	m_pChkLockQBar->Check(m_setting.bLockQuickBar);
	m_pChkHideIceThunderBall->Check(m_setting.bHideIceThunderBall);
	
	// Update new checkboxes
	if (m_pChkHideElf) m_pChkHideElf->Check(m_setting.bHideElf);
	if (m_pChkHideQuest) m_pChkHideQuest->Check(m_setting.bHideQuest);
	if (m_pChkBuff) m_pChkBuff->Check(m_setting.bNormalBuffDisplay);
	if (m_pChkUnfreeze) m_pChkUnfreeze->Check(m_setting.bUnfreeze);
	if (m_pChkOffFly) m_pChkOffFly->Check(m_setting.bOffFly);
	if (m_pChkOffHair) m_pChkOffHair->Check(m_setting.bOffHair);
	if (m_pChkNoDelay) m_pChkNoDelay->Check(m_setting.bNoDelay);
	if (m_pChkAutoComplete) m_pChkAutoComplete->Check(m_setting.bAutoComplete);
	if (m_pChkOffUnselect) m_pChkOffUnselect->Check(m_setting.bOffUnselect);
	if (m_pChkCDTimer) m_pChkCDTimer->Check(m_setting.bCDTimer);
	if (m_pChkOffWeapon) m_pChkOffWeapon->Check(m_setting.bOffWeapon);
	if (m_pChkFastFly) m_pChkFastFly->Check(m_setting.bFastFly);
	if (m_pChkOffBooth) m_pChkOffBooth->Check(m_setting.bOffBooth);
	
	// Load HP settings from UI settings (shared with optimize_gfx)
	const EC_UI_SETTING& uiSetting = GetGame()->GetConfigs()->GetUISettings();
	if (m_pChkHP) m_pChkHP->Check(uiSetting.bShowTargetHP);
	if (m_pChkTeamHP) m_pChkTeamHP->Check(uiSetting.bShowTeamHP);
}

void CDlgOptimizeFunc::OnCommand_Confirm(const char *szCommand) {
	CECGameUIMan* pGameUIMan = CECUIHelper::GetGameUIMan();
	pGameUIMan->SetShowItemDescCompare(!m_pChkEquipCompare->IsChecked());
	pGameUIMan->SetShowLowHP(!m_pChkLowHPGfx->IsChecked());
	pGameUIMan->SetShowTargetOfTarget(!m_pChkHideTargetOfTarget->IsChecked());

	m_setting.bPetAutoSkill = m_pChkPetBuff->IsChecked();
	m_setting.bLockQuickBar = m_pChkLockQBar->IsChecked();
	m_setting.bHideIceThunderBall = m_pChkHideIceThunderBall->IsChecked();
	
	// Save new settings
	if (m_pChkHideElf) m_setting.bHideElf = m_pChkHideElf->IsChecked();
	if (m_pChkHideQuest) m_setting.bHideQuest = m_pChkHideQuest->IsChecked();
	if (m_pChkBuff) m_setting.bNormalBuffDisplay = m_pChkBuff->IsChecked();
	if (m_pChkUnfreeze) m_setting.bUnfreeze = m_pChkUnfreeze->IsChecked();
	if (m_pChkOffFly) m_setting.bOffFly = m_pChkOffFly->IsChecked();
	if (m_pChkOffHair) m_setting.bOffHair = m_pChkOffHair->IsChecked();
	if (m_pChkNoDelay) m_setting.bNoDelay = m_pChkNoDelay->IsChecked();
	if (m_pChkAutoComplete) m_setting.bAutoComplete = m_pChkAutoComplete->IsChecked();
	if (m_pChkOffUnselect) m_setting.bOffUnselect = m_pChkOffUnselect->IsChecked();
	if (m_pChkCDTimer) m_setting.bCDTimer = m_pChkCDTimer->IsChecked();
	if (m_pChkOffWeapon) m_setting.bOffWeapon = m_pChkOffWeapon->IsChecked();
	if (m_pChkFastFly) m_setting.bFastFly = m_pChkFastFly->IsChecked();
	
	GetGame()->GetConfigs()->SetGameSettings_OptimizeFunction(m_setting);
	
	// Save settings to server when closing dialog
	if (g_pGame && g_pGame->GetGameRun())
		g_pGame->GetGameRun()->SaveConfigsToServer();

	CDlgIceThunderBall *pDlgIceThunderBall = static_cast<CDlgIceThunderBall *>(pGameUIMan->GetDialog("Win_IceThunderBall"));
	if( m_setting.bHideIceThunderBall ){
		pDlgIceThunderBall->Show(false);
	}else{
		if( pDlgIceThunderBall->GetCurrentState() != 0 ){
			pDlgIceThunderBall->Show(true);
			pDlgIceThunderBall->ShowCurrentBall(pDlgIceThunderBall->GetCurrentState());
		}
	}
	Show(false);
}

void CDlgOptimizeFunc::OnCommand_SelectAll(const char *szCommand) {
	m_pChkLowHPGfx->Check(true);
	m_pChkEquipCompare->Check(true);
	m_pChkLockQBar->Check(true);
	m_pChkPetBuff->Check(true);
	m_pChkHideIceThunderBall->Check(true);
	m_pChkHideTargetOfTarget->Check(true);
	if (m_pChkHideElf) m_pChkHideElf->Check(true);
	if (m_pChkHideQuest) m_pChkHideQuest->Check(true);
	if (m_pChkBuff) m_pChkBuff->Check(true);
	if (m_pChkUnfreeze) m_pChkUnfreeze->Check(true);
	if (m_pChkOffFly) m_pChkOffFly->Check(true);
	if (m_pChkOffHair) m_pChkOffHair->Check(true);
	if (m_pChkNoDelay) m_pChkNoDelay->Check(true);
	if (m_pChkAutoComplete) m_pChkAutoComplete->Check(true);
	if (m_pChkOffUnselect) m_pChkOffUnselect->Check(true);
	if (m_pChkCDTimer) m_pChkCDTimer->Check(true);
	if (m_pChkOffWeapon) m_pChkOffWeapon->Check(true);
	if (m_pChkFastFly) m_pChkFastFly->Check(true);
	if (m_pChkOffBooth) m_pChkOffBooth->Check(true);
	if (m_pChkHP) m_pChkHP->Check(true);
	if (m_pChkTeamHP) m_pChkTeamHP->Check(true);
}

void CDlgOptimizeFunc::OnCommand_Clear(const char *szCommand) {
	m_pChkLowHPGfx->Check(false);
	m_pChkEquipCompare->Check(false);
	m_pChkLockQBar->Check(false);
	m_pChkPetBuff->Check(false);
	m_pChkHideIceThunderBall->Check(false);
	m_pChkHideTargetOfTarget->Check(false);
	if (m_pChkHideElf) m_pChkHideElf->Check(false);
	if (m_pChkHideQuest) m_pChkHideQuest->Check(false);
	if (m_pChkBuff) m_pChkBuff->Check(false);
	if (m_pChkUnfreeze) m_pChkUnfreeze->Check(false);
	if (m_pChkOffFly) m_pChkOffFly->Check(false);
	if (m_pChkOffHair) m_pChkOffHair->Check(false);
	if (m_pChkNoDelay) m_pChkNoDelay->Check(false);
	if (m_pChkAutoComplete) m_pChkAutoComplete->Check(false);
	if (m_pChkOffUnselect) m_pChkOffUnselect->Check(false);
	if (m_pChkCDTimer) m_pChkCDTimer->Check(false);
	if (m_pChkOffWeapon) m_pChkOffWeapon->Check(false);
	if (m_pChkFastFly) m_pChkFastFly->Check(false);
	if (m_pChkOffBooth) m_pChkOffBooth->Check(false);
	if (m_pChkHP) m_pChkHP->Check(false);
	if (m_pChkTeamHP) m_pChkTeamHP->Check(false);
}

void CDlgOptimizeFunc::SetLockQBar(bool bLock) {
	m_pChkLockQBar->Check(bLock);
}

void CDlgOptimizeFunc::OnCommand_RadiusName(const char *szCommand)
{
	GetGame()->GetConfigs()->ToggleNameRenderRadius();
}

void CDlgOptimizeFunc::OnCommand_RadiusModel(const char *szCommand)
{
	GetGame()->GetConfigs()->ToggleModelRenderRadius();
}

// Command handlers for kwi_ commands (these update settings immediately)
void CDlgOptimizeFunc::OnCommand_Unfreeze(const char *szCommand)
{
	if (m_pChkUnfreeze) {
		m_setting.bUnfreeze = m_pChkUnfreeze->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Apply bUnfreeze setting immediately
		g_bRenderNoFocus = m_setting.bUnfreeze;
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_OffFly(const char *szCommand)
{
	if (m_pChkOffFly) {
		m_setting.bOffFly = m_pChkOffFly->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_OffHair(const char *szCommand)
{
	if (m_pChkOffHair) {
		m_setting.bOffHair = m_pChkOffHair->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_NoDelay(const char *szCommand)
{
	if (m_pChkNoDelay) {
		m_setting.bNoDelay = m_pChkNoDelay->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_AutoComplete(const char *szCommand)
{
	if (m_pChkAutoComplete) {
		m_setting.bAutoComplete = m_pChkAutoComplete->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_OffUnselect(const char *szCommand)
{
	if (m_pChkOffUnselect) {
		m_setting.bOffUnselect = m_pChkOffUnselect->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_CDTimer(const char *szCommand)
{
	if (m_pChkCDTimer) {
		m_setting.bCDTimer = m_pChkCDTimer->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_OffWeapon(const char *szCommand)
{
	if (m_pChkOffWeapon) {
		m_setting.bOffWeapon = m_pChkOffWeapon->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_FastFly(const char *szCommand)
{
	if (m_pChkFastFly) {
		m_setting.bFastFly = m_pChkFastFly->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_OffBooth(const char *szCommand)
{
	if (m_pChkOffBooth) {
		m_setting.bOffBooth = m_pChkOffBooth->IsChecked();
		GetGame()->GetConfigs()->SetGameSettings(m_setting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_HP(const char *szCommand)
{
	if (m_pChkHP) {
		EC_UI_SETTING uiSetting = GetGame()->GetConfigs()->GetUISettings();
		uiSetting.bShowTargetHP = m_pChkHP->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(uiSetting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

void CDlgOptimizeFunc::OnCommand_TeamHP(const char *szCommand)
{
	if (m_pChkTeamHP) {
		EC_UI_SETTING uiSetting = GetGame()->GetConfigs()->GetUISettings();
		uiSetting.bShowTeamHP = m_pChkTeamHP->IsChecked();
		GetGame()->GetConfigs()->SetUISettings(uiSetting);
		// Save settings to server
		if (g_pGame && g_pGame->GetGameRun())
			g_pGame->GetGameRun()->SaveConfigsToServer();
	}
}

