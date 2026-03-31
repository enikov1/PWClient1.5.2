// Filename	: DlgOptimizeUI.h
// Creator	: Auto-generated
// Date		: 2024

#pragma once

#include "DlgOptimizeBase.h"
#include "EC_Configs.h"

class AUISlider;
class AUICheckBox;
class AUIObject;

class CDlgOptimizeUI : public CDlgOptimizeBase
{
	AUI_DECLARE_COMMAND_MAP();
    
	// Sliders for UI scales
	AUISlider* m_pSliderTarget;
	AUISlider* m_pSliderMap;
	AUISlider* m_pSliderQuickbar;
	AUISlider* m_pSliderChat;
	AUISlider* m_pSliderTeam;
	AUISlider* m_pSliderElf;
	AUISlider* m_pSliderAmulet;
	AUISlider* m_pSliderInventory;
	AUISlider* m_pSliderOption;
	AUISlider* m_pSliderOther;
	
	// Labels for current scale values
	AUIObject* m_pTxtCurTarget;
	AUIObject* m_pTxtCurMap;
	AUIObject* m_pTxtCurQuickbar;
	AUIObject* m_pTxtCurChat;
	AUIObject* m_pTxtCurTeam;
	AUIObject* m_pTxtCurElf;
	AUIObject* m_pTxtCurAmulet;
	AUIObject* m_pTxtCurInventory;
	AUIObject* m_pTxtCurOption;
	AUIObject* m_pTxtCurOther;
	
	// Checkboxes for UI features
	AUICheckBox* m_pChkTargetHP;
	AUICheckBox* m_pChkTeamHP;
	AUICheckBox* m_pChkElfMode;
	AUICheckBox* m_pChkHideTitle;
	AUICheckBox* m_pChkExtraPanelSize;
	AUICheckBox* m_pChkShowTargetCast;
	AUICheckBox* m_pChkSetCustomWindowTitle;

	EC_UI_SETTING m_uiSetting;
        
protected:
	virtual bool OnInitDialog();
	virtual void OnShowDialog();
    
	void OnCommand_Apply(const char *szCommand);
	void OnCommand_Confirm(const char *szCommand);
	void OnCommand_UIDefault(const char *szCommand);
	
	void OnCommand_SliderTarget(const char *szCommand);
	void OnCommand_SliderMap(const char *szCommand);
	void OnCommand_SliderQuickbar(const char *szCommand);
	void OnCommand_SliderChat(const char *szCommand);
	void OnCommand_SliderTeam(const char *szCommand);
	void OnCommand_SliderElf(const char *szCommand);
	void OnCommand_SliderAmulet(const char *szCommand);
	void OnCommand_SliderInventory(const char *szCommand);
	void OnCommand_SliderOption(const char *szCommand);
	void OnCommand_SliderOther(const char *szCommand);
	
	void OnCommand_kwi_targethp(const char *szCommand);
	void OnCommand_kwi_teamhp(const char *szCommand);
	void OnCommand_kwi_elfmode(const char *szCommand);
	void OnCommand_kwi_title_enable(const char *szCommand);
	void OnCommand_kwi_extrapanelsize(const char *szCommand);
	void OnCommand_kwi_targetcastskill(const char *szCommand);
	void OnCommand_kwi_setcustomwnd(const char *szCommand);

	void UpdateUIView();
	void UpdateScaleLabel(AUIObject* pLabel, BYTE level);
	void ApplyUIScales();

public:
	CDlgOptimizeUI();
	virtual ~CDlgOptimizeUI();
};

