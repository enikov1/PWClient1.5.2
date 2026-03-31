// Filename	: DlgPKSetting.h
// Creator	: Xiao Zhou
// Date		: 2005/11/18

#pragma once

#include "DlgBase.h"
#include "AUICheckBox.h"
#include "AUISlider.h"

class CDlgPKSetting : public CDlgBase  
{
	AUI_DECLARE_COMMAND_MAP()

	void OnCommand_Confirm(const char * szCommand);
	void OnCommand_NewTab(const char * szCommand);
	void OnCommand_TabClass(int nClass);
	void OnCommand_TabClass0(const char * szCommand);
	void OnCommand_TabClass1(const char * szCommand);
	void OnCommand_TabClass2(const char * szCommand);
	void OnCommand_TabClass3(const char * szCommand);
	void OnCommand_TabClass4(const char * szCommand);
	void OnCommand_TabClass5(const char * szCommand);
	void OnCommand_TabClass6(const char * szCommand);
	void OnCommand_TabClass7(const char * szCommand);
	void OnCommand_TabClass8(const char * szCommand);
	void OnCommand_TabClass9(const char * szCommand);
	void OnCommand_TabClass10(const char * szCommand);
	void OnCommand_TabClass11(const char * szCommand);
	void OnCommand_TabClass12(const char * szCommand);
	void OnCommand_TabClass13(const char * szCommand);
	void OnCommand_TabClass14(const char * szCommand);
	void OnCommand_AddTabRadius(const char * szCommand);
	void OnCommand_DecTabRadius(const char * szCommand);
	void OnCommand_SldRadius(const char * szCommand);

public:
	CDlgPKSetting();
	virtual ~CDlgPKSetting();

	void OnChangePVPNoPenalty(bool bEnter);	
	void OnCommand_pk(const char * szCommand);

protected:
	virtual bool OnInitDialog();
	virtual void OnShowDialog();
	virtual void OnTick();
	
	void UpdateRadiusDisplay();
	void UpdateClassFilterCheckboxes();
	
	int m_iTabSelMode; // Radio button selection (0=Normal, 1=Monster, 2=Player)
	
	PAUICHECKBOX		m_pChk_Gprotect;
	PAUICHECKBOX		m_pChk_Wprotect;
	PAUICHECKBOX		m_pChk_Aprotect;
	PAUICHECKBOX		m_pChk_Allianceprotect;
	PAUICHECKBOX		m_pChk_Fprotect; // 势力保护
	PAUICHECKBOX		m_pChk_Buff1;
	PAUICHECKBOX		m_pChk_Buff2;
	PAUICHECKBOX		m_pChk_Buff3;
	PAUICHECKBOX		m_pChk_Buff4;
	PAUICHECKBOX		m_pChk_Buff5;
	PAUICHECKBOX		m_pChk_Buff6;
	PAUICHECKBOX		m_pChk_BlsNoForce; // 势力屏蔽
	PAUICHECKBOX		m_pChk_SwitchPK;
	PAUIOBJECT			m_pTxt_SwitchPK;
	
	// New controls
	PAUICHECKBOX		m_pChkNewTab;			// kwi_newtab
	PAUICHECKBOX		m_pChkTabClass[15];		// kwi_tabclass0-14
	PAUISLIDER			m_pSliderRadius;		// Sld_Radius
	AUIObject*			m_pEditDist;			// edi_dist (readonly edit box)
	PAUIOBJECT			m_pTxtRadius;			// Txt_Radius
};
