// Filename	: DlgPKSetting.cpp
// Creator	: Xiao Zhou
// Date		: 2005/11/18

#include "DlgPKSetting.h"
#include "DlgSetting.h"
#include "EC_Game.h"
#include "EC_Configs.h"
#include "EC_GameUIMan.h"
#include "EC_Utility.h"
#include "EC_GameSession.h"
#include "EC_HostPlayer.h"
#include "EC_GameRun.h"
#include <AUISlider.h>
#include <string.h>

#define new A_DEBUG_NEW

AUI_BEGIN_COMMAND_MAP(CDlgPKSetting, CDlgBase)

AUI_ON_COMMAND("confirm",		OnCommand_Confirm)
AUI_ON_COMMAND("Chk_SwitchPK",	OnCommand_pk)
AUI_ON_COMMAND("kwi_newtab",	OnCommand_NewTab)
AUI_ON_COMMAND("kwi_addtabradius", OnCommand_AddTabRadius)
AUI_ON_COMMAND("kwi_dectabradius", OnCommand_DecTabRadius)
AUI_ON_COMMAND("Sld_Radius",	OnCommand_SldRadius)
AUI_ON_COMMAND("kwi_tabclass0", OnCommand_TabClass0)
AUI_ON_COMMAND("kwi_tabclass1", OnCommand_TabClass1)
AUI_ON_COMMAND("kwi_tabclass2", OnCommand_TabClass2)
AUI_ON_COMMAND("kwi_tabclass3", OnCommand_TabClass3)
AUI_ON_COMMAND("kwi_tabclass4", OnCommand_TabClass4)
AUI_ON_COMMAND("kwi_tabclass5", OnCommand_TabClass5)
AUI_ON_COMMAND("kwi_tabclass6", OnCommand_TabClass6)
AUI_ON_COMMAND("kwi_tabclass7", OnCommand_TabClass7)
AUI_ON_COMMAND("kwi_tabclass8", OnCommand_TabClass8)
AUI_ON_COMMAND("kwi_tabclass9", OnCommand_TabClass9)
AUI_ON_COMMAND("kwi_tabclass10", OnCommand_TabClass10)
AUI_ON_COMMAND("kwi_tabclass11", OnCommand_TabClass11)
AUI_ON_COMMAND("kwi_tabclass12", OnCommand_TabClass12)
AUI_ON_COMMAND("kwi_tabclass13", OnCommand_TabClass13)
AUI_ON_COMMAND("kwi_tabclass14", OnCommand_TabClass14)

AUI_END_COMMAND_MAP()

CDlgPKSetting::CDlgPKSetting()
	: m_iTabSelMode(0)
{
	m_pChk_Aprotect = NULL;
	m_pChk_Gprotect = NULL;
	m_pChk_Wprotect = NULL;
	m_pChk_Allianceprotect = NULL;
	m_pChk_Fprotect = NULL;
	m_pChk_Buff1 = NULL;
	m_pChk_Buff2 = NULL;
	m_pChk_Buff3 = NULL;
	m_pChk_Buff4 = NULL;
	m_pChk_Buff5 = NULL;
	m_pChk_Buff6 = NULL;
	m_pChk_BlsNoForce = NULL;
	m_pChk_SwitchPK = NULL;
	m_pTxt_SwitchPK = NULL;
	
	m_pChkNewTab = NULL;
	memset(m_pChkTabClass, 0, sizeof(m_pChkTabClass));
	m_pSliderRadius = NULL;
	m_pEditDist = NULL;
	m_pTxtRadius = NULL;
}

CDlgPKSetting::~CDlgPKSetting()
{
}

void CDlgPKSetting::OnCommand_Confirm(const char * szCommand)
{
	EC_GAME_SETTING gs = GetGame()->GetConfigs()->GetGameSettings();
	gs.bAtk_Player = !m_pChk_Aprotect->IsChecked();
	gs.bAtk_NoMafia = m_pChk_Gprotect->IsChecked();
	gs.bAtk_NoWhite = m_pChk_Wprotect->IsChecked();
	gs.bAtk_NoAlliance = m_pChk_Allianceprotect->IsChecked();
	gs.bAtk_NoForce = m_pChk_Fprotect->IsChecked();
	gs.bBls_NoRed = m_pChk_Buff1->IsChecked();
	gs.bBls_NoMafia = m_pChk_Buff2->IsChecked();
	gs.bBls_Self = m_pChk_Buff3->IsChecked();
	gs.bBlsRefuse_Neutral = m_pChk_Buff4->IsChecked();
	gs.bBls_NoAlliance = m_pChk_Buff5->IsChecked();
	gs.bBlsRefuse_NonTeammate = m_pChk_Buff6->IsChecked();
	gs.bBls_NoForce = m_pChk_BlsNoForce->IsChecked();

	GetGame()->GetConfigs()->SetGameSettings(gs);
	Show(false);
	GetGameUIMan()->m_pDlgSettingCurrent->UpdateView();

	BYTE forceAttack = glb_BuildPVPMask(false);
	BYTE refuseBless = glb_BuildRefuseBLSMask();
	GetGameSession()->c2s_CmdNotifyForceAttack(forceAttack, refuseBless);

	// Save TAB selection mode
	int nMode(0);
	m_iTabSelMode = GetCheckedRadioButton(0); // Get current radio button selection
	switch (m_iTabSelMode)
	{
	case 0: nMode = CECHostPlayer::TSL_NORMAL; break;
	case 1: nMode = CECHostPlayer::TSL_MONSTER; break;
	case 2: nMode = CECHostPlayer::TSL_PLAYER; break;
	default: ASSERT(false); break;
	}	
	CECConfigs *pConfigs = GetGame()->GetConfigs();
	EC_VIDEO_SETTING vs = pConfigs->GetVideoSettings();
	
	// Save TAB selection settings
	vs.cTabSelType = (char)nMode;
	if (m_pSliderRadius)
	{
		// Convert slider level (0-50) to radius (0.0-100.0 meters)
		BYTE level = (BYTE)m_pSliderRadius->GetLevel();
		vs.fTabSelRadius = level * 2.0f; // 0-50 -> 0.0-100.0
	}
	if (m_pChkNewTab)
		vs.bTabClassFilterEnabled = m_pChkNewTab->IsChecked();
	
	// Build class filter mask
	vs.dwTabClassFilter = 0;
	for (int i = 0; i < 15; i++)
	{
		if (m_pChkTabClass[i] && m_pChkTabClass[i]->IsChecked())
			vs.dwTabClassFilter |= (1UL << i);
	}
	
	if (vs.cTabSelType != (char)nMode || (m_pSliderRadius && vs.fTabSelRadius != pConfigs->GetVideoSettings().fTabSelRadius) ||
		(m_pChkNewTab && vs.bTabClassFilterEnabled != pConfigs->GetVideoSettings().bTabClassFilterEnabled) ||
		vs.dwTabClassFilter != pConfigs->GetVideoSettings().dwTabClassFilter)
	{
		pConfigs->SetVideoSettings(vs);
		GetHostPlayer()->ResetAutoSelMap();
	}
}

void CDlgPKSetting::OnCommand_pk(const char * szCommand)
{
	const CECPlayer::PVPINFO &pvp = GetHostPlayer()->GetPVPInfo();
	
	if( pvp.bEnable && pvp.dwCoolTime == 0 )
	{
		GetGameUIMan()->MessageBox("Game_PVPClose", GetStringFromTable(561),
			MB_OKCANCEL, A3DCOLORRGBA(255, 255, 255, 160));
	}
	else if( !pvp.bEnable )
	{
		if( GetHostPlayer()->GetBasicProps().iLevel < 30 )
		{
			GetGameUIMan()->MessageBox("", GetStringFromTable(581),
				MB_OK, A3DCOLORRGBA(255, 255, 255, 160));
		}
		else
		{
			GetGameUIMan()->MessageBox("Game_PVPOpen", GetStringFromTable(542),
				MB_OKCANCEL, A3DCOLORRGBA(255, 255, 255, 160));
		}
	}

	//	玩家点击后，选中状态被改变、并导致图标切换，但实际上不一定会切换，
	//	此处改回原图，如果切换成功，下次 OnShowDialog 时更新
	m_pChk_SwitchPK->Check(!m_pChk_SwitchPK->IsChecked());
}

bool CDlgPKSetting::OnInitDialog()
{
	DDX_Control("Chk_Aprotect", m_pChk_Aprotect);
	DDX_Control("Chk_Gprotect", m_pChk_Gprotect);
	DDX_Control("Chk_Wprotect", m_pChk_Wprotect);
	DDX_Control("Chk_Allianceprotect", m_pChk_Allianceprotect);
	DDX_Control("Chk_Other",m_pChk_Fprotect);

	DDX_Control("Chk_Buff1", m_pChk_Buff1);
	DDX_Control("Chk_Buff2", m_pChk_Buff2);
	DDX_Control("Chk_Buff3", m_pChk_Buff3);
	DDX_Control("Chk_Buff4", m_pChk_Buff4);
	DDX_Control("Chk_Buff5", m_pChk_Buff5);
	DDX_Control("Chk_Buff6", m_pChk_Buff6);
	DDX_Control("Chk_Buff7", m_pChk_BlsNoForce);

	DDX_Control("Chk_SwitchPK", m_pChk_SwitchPK);
	DDX_Control("Txt_SwitchPK", m_pTxt_SwitchPK);
	
	// Radio buttons are handled via CheckRadioButton/GetCheckedRadioButton, no DDX needed
	
	// New controls
	DDX_Control("kwi_newtab", m_pChkNewTab);
	DDX_Control("kwi_tabclass0", m_pChkTabClass[0]);
	DDX_Control("kwi_tabclass1", m_pChkTabClass[1]);
	DDX_Control("kwi_tabclass2", m_pChkTabClass[2]);
	DDX_Control("kwi_tabclass3", m_pChkTabClass[3]);
	DDX_Control("kwi_tabclass4", m_pChkTabClass[4]);
	DDX_Control("kwi_tabclass5", m_pChkTabClass[5]);
	DDX_Control("kwi_tabclass6", m_pChkTabClass[6]);
	DDX_Control("kwi_tabclass7", m_pChkTabClass[7]);
	DDX_Control("kwi_tabclass8", m_pChkTabClass[8]);
	DDX_Control("kwi_tabclass9", m_pChkTabClass[9]);
	DDX_Control("kwi_tabclass10", m_pChkTabClass[10]);
	DDX_Control("kwi_tabclass11", m_pChkTabClass[11]);
	DDX_Control("kwi_tabclass12", m_pChkTabClass[12]);
	DDX_Control("kwi_tabclass13", m_pChkTabClass[13]);
	DDX_Control("kwi_tabclass14", m_pChkTabClass[14]);
	DDX_Control("Sld_Radius", m_pSliderRadius);
	DDX_Control("edi_dist", m_pEditDist);
	DDX_Control("Txt_Radius", m_pTxtRadius);

	return true;
}

void CDlgPKSetting::OnShowDialog()
{
	const EC_GAME_SETTING& gs = GetGame()->GetConfigs()->GetGameSettings();

	m_pChk_Aprotect->Check(!gs.bAtk_Player);
	m_pChk_Gprotect->Check(gs.bAtk_NoMafia);
	m_pChk_Wprotect->Check(gs.bAtk_NoWhite);
	m_pChk_Allianceprotect->Check(gs.bAtk_NoAlliance);
	m_pChk_Fprotect->Check(gs.bAtk_NoForce);
	m_pChk_Buff1->Check(gs.bBls_NoRed);
	m_pChk_Buff2->Check(gs.bBls_NoMafia);
	m_pChk_Buff3->Check(gs.bBls_Self);
	m_pChk_Buff4->Check(gs.bBlsRefuse_Neutral);
	m_pChk_Buff5->Check(gs.bBls_NoAlliance);
	m_pChk_Buff6->Check(gs.bBlsRefuse_NonTeammate);
	m_pChk_BlsNoForce->Check(gs.bBls_NoForce);
	
	const EC_VIDEO_SETTING& vs = GetGame()->GetConfigs()->GetVideoSettings();	
	m_iTabSelMode = vs.cTabSelType;
	CheckRadioButton(0, vs.cTabSelType);
	
	// Update new controls
	if (m_pChkNewTab)
		m_pChkNewTab->Check(vs.bTabClassFilterEnabled);
	
	UpdateClassFilterCheckboxes();
	UpdateRadiusDisplay();	
	
	if( GetGameRun()->GetPVPMode() )
	{
		m_pChk_SwitchPK->Show(false);
		m_pTxt_SwitchPK->Show(false);
	}
	else
	{
		const CECPlayer::PVPINFO &pvp = GetHostPlayer()->GetPVPInfo();	
		m_pTxt_SwitchPK->Show(true);
		m_pChk_SwitchPK->Show(true);
		m_pChk_SwitchPK->Check(pvp.bEnable);
		
		if( pvp.bEnable )
		{			
			ACString strText;
			bool bRed = GetHostPlayer()->IsPariah() || GetHostPlayer()->IsInvader();
			if( bRed )
			{
				if( GetHostPlayer()->IsPariah() )
					strText.Format(GetStringFromTable(834), GetHostPlayer()->GetPariahTime() / 3600000 + 1);
				else
					strText = GetStringFromTable(615);
			}
			else if( pvp.dwCoolTime > 0 )
				strText.Format(GetStringFromTable(545), pvp.dwCoolTime / 3600000 + 1);
			else
				strText = GetStringFromTable(614);
			m_pChk_SwitchPK->SetHint(strText);
			m_pChk_SwitchPK->Enable(pvp.dwCoolTime > 0 || bRed ? false : true);
		}
		else
		{
			m_pChk_SwitchPK->Enable(true);
			m_pChk_SwitchPK->SetHint(GetStringFromTable(547));
		}
	}
}

void CDlgPKSetting::OnTick()
{
	m_pChk_Gprotect->Enable(!m_pChk_Aprotect->IsChecked());
	m_pChk_Wprotect->Enable(!m_pChk_Aprotect->IsChecked());
	m_pChk_Allianceprotect->Enable(!m_pChk_Aprotect->IsChecked());
	if( m_pChk_Aprotect->IsChecked() )
	{
		m_pChk_Gprotect->Check(true);
		m_pChk_Wprotect->Check(true);
		m_pChk_Allianceprotect->Check(true);
	}
}
void CDlgPKSetting::OnChangePVPNoPenalty(bool bEnter)
{
	m_pChk_Fprotect->Check(bEnter);
	m_pChk_BlsNoForce->Check(bEnter);

	EC_GAME_SETTING gs = GetGame()->GetConfigs()->GetGameSettings();
	gs.bAtk_NoForce = bEnter;
	gs.bBls_NoForce = bEnter;

	GetGame()->GetConfigs()->SetGameSettings(gs);
// 	Show(false);
 	GetGameUIMan()->m_pDlgSettingCurrent->UpdateView();
	
	BYTE forceAttack = glb_BuildPVPMask(false);
	BYTE refuseBless = glb_BuildRefuseBLSMask();
	GetGameSession()->c2s_CmdNotifyForceAttack(forceAttack, refuseBless);
}

void CDlgPKSetting::UpdateRadiusDisplay()
{
	if (!m_pSliderRadius || !m_pEditDist || !m_pTxtRadius)
		return;
	
	const EC_VIDEO_SETTING& vs = GetGame()->GetConfigs()->GetVideoSettings();
	float fRadius = vs.fTabSelRadius;
	
	// Convert radius (0.0-100.0) to slider level (0-50)
	BYTE level = (BYTE)(fRadius / 2.0f + 0.5f);
	if (level > 50) level = 50;
	m_pSliderRadius->SetLevel(level);
	
	// Update edit box
	ACString strEdit;
	strEdit.Format(L"%.0f", fRadius);
	m_pEditDist->SetText(strEdit);
	
	// Update label
	ACString strLabel;
	strLabel.Format(L"%.1f м", fRadius);
	m_pTxtRadius->SetText(strLabel);
}

void CDlgPKSetting::UpdateClassFilterCheckboxes()
{
	const EC_VIDEO_SETTING& vs = GetGame()->GetConfigs()->GetVideoSettings();
	DWORD dwFilter = vs.dwTabClassFilter;
	
	for (int i = 0; i < 15; i++)
	{
		if (m_pChkTabClass[i])
		{
			bool bChecked = (dwFilter & (1UL << i)) != 0;
			m_pChkTabClass[i]->Check(bChecked);
		}
	}
}

void CDlgPKSetting::OnCommand_NewTab(const char * szCommand)
{
	if (m_pChkNewTab)
	{
		EC_VIDEO_SETTING vs = GetGame()->GetConfigs()->GetVideoSettings();
		vs.bTabClassFilterEnabled = m_pChkNewTab->IsChecked();
		GetGame()->GetConfigs()->SetVideoSettings(vs);
	}
}

void CDlgPKSetting::OnCommand_AddTabRadius(const char * szCommand)
{
	if (!m_pSliderRadius)
		return;
	
	BYTE currentLevel = (BYTE)m_pSliderRadius->GetLevel();
	if (currentLevel < 50) // Slider has 50 levels (0-50) as defined in XML
	{
		m_pSliderRadius->SetLevel(currentLevel + 1);
		OnCommand_SldRadius(szCommand);
	}
}

void CDlgPKSetting::OnCommand_DecTabRadius(const char * szCommand)
{
	if (!m_pSliderRadius)
		return;
	
	BYTE currentLevel = (BYTE)m_pSliderRadius->GetLevel();
	if (currentLevel > 0)
	{
		m_pSliderRadius->SetLevel(currentLevel - 1);
		OnCommand_SldRadius(szCommand);
	}
}

void CDlgPKSetting::OnCommand_SldRadius(const char * szCommand)
{
	if (!m_pSliderRadius)
		return;
	
	EC_VIDEO_SETTING vs = GetGame()->GetConfigs()->GetVideoSettings();
	BYTE level = (BYTE)m_pSliderRadius->GetLevel();
	vs.fTabSelRadius = level * 2.0f; // 0-50 -> 0.0-100.0
	GetGame()->GetConfigs()->SetVideoSettings(vs);
	
	UpdateRadiusDisplay();
}

void CDlgPKSetting::OnCommand_TabClass0(const char * szCommand) { OnCommand_TabClass(0); }
void CDlgPKSetting::OnCommand_TabClass1(const char * szCommand) { OnCommand_TabClass(1); }
void CDlgPKSetting::OnCommand_TabClass2(const char * szCommand) { OnCommand_TabClass(2); }
void CDlgPKSetting::OnCommand_TabClass3(const char * szCommand) { OnCommand_TabClass(3); }
void CDlgPKSetting::OnCommand_TabClass4(const char * szCommand) { OnCommand_TabClass(4); }
void CDlgPKSetting::OnCommand_TabClass5(const char * szCommand) { OnCommand_TabClass(5); }
void CDlgPKSetting::OnCommand_TabClass6(const char * szCommand) { OnCommand_TabClass(6); }
void CDlgPKSetting::OnCommand_TabClass7(const char * szCommand) { OnCommand_TabClass(7); }
void CDlgPKSetting::OnCommand_TabClass8(const char * szCommand) { OnCommand_TabClass(8); }
void CDlgPKSetting::OnCommand_TabClass9(const char * szCommand) { OnCommand_TabClass(9); }
void CDlgPKSetting::OnCommand_TabClass10(const char * szCommand) { OnCommand_TabClass(10); }
void CDlgPKSetting::OnCommand_TabClass11(const char * szCommand) { OnCommand_TabClass(11); }
void CDlgPKSetting::OnCommand_TabClass12(const char * szCommand) { OnCommand_TabClass(12); }
void CDlgPKSetting::OnCommand_TabClass13(const char * szCommand) { OnCommand_TabClass(13); }
void CDlgPKSetting::OnCommand_TabClass14(const char * szCommand) { OnCommand_TabClass(14); }

void CDlgPKSetting::OnCommand_TabClass(int nClass)
{
	if (nClass < 0 || nClass >= 15 || !m_pChkTabClass[nClass])
		return;
	
	EC_VIDEO_SETTING vs = GetGame()->GetConfigs()->GetVideoSettings();
	DWORD dwFilter = vs.dwTabClassFilter;
	
	if (m_pChkTabClass[nClass]->IsChecked())
		dwFilter |= (1UL << nClass);
	else
		dwFilter &= ~(1UL << nClass);
	
	vs.dwTabClassFilter = dwFilter;
	GetGame()->GetConfigs()->SetVideoSettings(vs);
}
