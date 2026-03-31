// Filename	: DlgOptimizeMem.h
// Creator	: Xu Wenbin
// Date		: 2013/4/17

#pragma once

#include "DlgOptimizeBase.h"
#include "EC_MemSimplify.h"
#include "EC_Configs.h"
#include <AUICheckBox.h>
#include <AUISlider.h>
#include "AUIRadioButton.h"

class AUICheckBox;
class AUISlider;

class CDlgOptimizeMem : public CDlgOptimizeBase
{
	friend class CDlgSettingVideo;

	AUI_DECLARE_COMMAND_MAP();

	bool m_bAutoOptimize;	// 自动优化
	int m_iMemState;		// 手动设置优化级别
	EC_OPTIMIZE_SETTING m_setting; // Memory optimization settings

protected:
	virtual bool OnInitDialog();
	virtual void OnShowDialog();
	virtual void DoDataExchange(bool bSave);

    void UpdateMemView();
	void UpdateSettingsView();
	void UpdateValueLabels();
	void ApplySettings();
	
	// Radio buttons
	PAUIRADIOBUTTON m_pRdoNormal;
	PAUIRADIOBUTTON m_pRdoQuick;
	
	// Sliders
	AUISlider* m_pSliderModelLimit;
	AUISlider* m_pSliderMateLimit;
	AUISlider* m_pSliderEnemyLimit;
	AUISlider* m_pSliderSpEffect;
	AUISlider* m_pSliderEffectsDistance;
	AUISlider* m_pSliderEffectsType;
	AUISlider* m_pSliderNameDistance;
	
	// Checkboxes
	PAUICHECKBOX m_pChkPeopleLimit;
	PAUICHECKBOX m_pChkModelLimit;
	PAUICHECKBOX m_pChkPeopleLimitByNear;
	PAUICHECKBOX m_pChkSpEffect;
	PAUICHECKBOX m_pChkEffectsDistance;
	PAUICHECKBOX m_pChkEffectsShow;
	PAUICHECKBOX m_pChkExcludeHost;
	PAUICHECKBOX m_pChkTargetSimpleModel;
	
	// Labels for values
	AUIObject* m_pTxtModelLimitValue;
	AUIObject* m_pTxtMateLimitValue;
	AUIObject* m_pTxtEnemyLimitValue;
	AUIObject* m_pTxtEffectsDistanceValue;
	AUIObject* m_pTxtDist;
	AUIObject* m_pEditDist;
	
	// Old checkboxes (kept for compatibility)
	PAUICHECKBOX m_pChk_Player;
	PAUICHECKBOX m_pChk_Surround;
	PAUICHECKBOX m_pChk_Sight;

	// Command handlers
	void OnCommand_Confirm(const char *szCommand);
	void OnCommand_OptimizeMem(const char* szCommand);
	void OnCommand_Apply(const char *szCommand);
	void OnCommand_Default(const char *szCommand);
	void OnCommand_ModeChanged(const char *szCommand);
	void OnCommand_ModelLimit(const char *szCommand);
	void OnCommand_MateLimit(const char *szCommand);
	void OnCommand_EnemyLimit(const char *szCommand);
	void OnCommand_SpEffect(const char *szCommand);
	void OnCommand_EffectsDistance(const char *szCommand);
	void OnCommand_EffectsType(const char *szCommand);
	void OnCommand_SldDist(const char *szCommand);
	void OnCommand_AddDist(const char *szCommand);
	void OnCommand_DecDist(const char *szCommand);
	void OnCommand_TargetSimpleModel(const char *szCommand);

public:
	CDlgOptimizeMem();
	virtual ~CDlgOptimizeMem();
};

