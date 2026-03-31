// Filename	: DlgOptimizeMem.cpp
// Creator	: Xu Wenbin
// Date		: 2013/4/17

#include "DlgOptimizeMem.h"
#include "EC_Global.h"
#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EC_Configs.h"
#include "EC_GameUIMan.h"
#include "DlgSetting.h"
#include <AUISlider.h>

#define new A_DEBUG_NEW

AUI_BEGIN_COMMAND_MAP(CDlgOptimizeMem, CDlgOptimizeBase)
AUI_ON_COMMAND("Btn_Decide", OnCommand_Confirm)
AUI_ON_COMMAND("confirm", OnCommand_Confirm)
AUI_ON_COMMAND("Chk_OptimizeMem", OnCommand_OptimizeMem)
AUI_ON_COMMAND("Btn_Apply", OnCommand_Apply)
AUI_ON_COMMAND("apply", OnCommand_Apply)
AUI_ON_COMMAND("default", OnCommand_Default)
AUI_ON_COMMAND("modeChanged", OnCommand_ModeChanged)
AUI_ON_COMMAND("modelLimit", OnCommand_ModelLimit)
AUI_ON_COMMAND("mateLimit", OnCommand_MateLimit)
AUI_ON_COMMAND("enemyLimit", OnCommand_EnemyLimit)
AUI_ON_COMMAND("spEffect", OnCommand_SpEffect)
AUI_ON_COMMAND("effectsDistence", OnCommand_EffectsDistance)
AUI_ON_COMMAND("effectsType", OnCommand_EffectsType)
AUI_ON_COMMAND("Sld_Dist", OnCommand_SldDist)
AUI_ON_COMMAND("kwi_adddist", OnCommand_AddDist)
AUI_ON_COMMAND("kwi_decdist", OnCommand_DecDist)
AUI_ON_COMMAND("kwi_target_simplemodel", OnCommand_TargetSimpleModel)
AUI_END_COMMAND_MAP()


CDlgOptimizeMem::CDlgOptimizeMem()
	: m_bAutoOptimize(true)
	, m_iMemState(CECMemSimplify::MEMUSAGE_NORMAL)
	, m_pRdoNormal(NULL)
	, m_pRdoQuick(NULL)
	, m_pSliderModelLimit(NULL)
	, m_pSliderMateLimit(NULL)
	, m_pSliderEnemyLimit(NULL)
	, m_pSliderSpEffect(NULL)
	, m_pSliderEffectsDistance(NULL)
	, m_pSliderEffectsType(NULL)
	, m_pSliderNameDistance(NULL)
	, m_pChkPeopleLimit(NULL)
	, m_pChkModelLimit(NULL)
	, m_pChkPeopleLimitByNear(NULL)
	, m_pChkSpEffect(NULL)
	, m_pChkEffectsDistance(NULL)
	, m_pChkEffectsShow(NULL)
	, m_pChkExcludeHost(NULL)
	, m_pChkTargetSimpleModel(NULL)
	, m_pTxtModelLimitValue(NULL)
	, m_pTxtMateLimitValue(NULL)
	, m_pTxtEnemyLimitValue(NULL)
	, m_pTxtEffectsDistanceValue(NULL)
	, m_pTxtDist(NULL)
	, m_pEditDist(NULL)
	, m_pChk_Player(NULL)
	, m_pChk_Surround(NULL)
	, m_pChk_Sight(NULL)
{
}

CDlgOptimizeMem::~CDlgOptimizeMem()
{
}

bool CDlgOptimizeMem::OnInitDialog()
{
	// Radio buttons
	DDX_Control("Rdo_Normal", m_pRdoNormal);
	DDX_Control("Rdo_Quick", m_pRdoQuick);
	
	// Sliders
	DDX_Control("Slider_ModelLimit", m_pSliderModelLimit);
	DDX_Control("Slider_MateLimit", m_pSliderMateLimit);
	DDX_Control("Slider_EnemyLimit", m_pSliderEnemyLimit);
	DDX_Control("Slider_SpEffect", m_pSliderSpEffect);
	DDX_Control("Slider_EffectsDistence", m_pSliderEffectsDistance);
	DDX_Control("Slider_EffectsType", m_pSliderEffectsType);
	DDX_Control("Sld_Dist", m_pSliderNameDistance);
	
	// Checkboxes
	DDX_Control("Chk_PeopleLimit", m_pChkPeopleLimit);
	DDX_Control("Chk_ModelLimit", m_pChkModelLimit);
	DDX_Control("Chk_PeopleLimitByNear", m_pChkPeopleLimitByNear);
	DDX_Control("Chk_SpEffect", m_pChkSpEffect);
	DDX_Control("Chk_EffectsDistence", m_pChkEffectsDistance);
	DDX_Control("Chk_EffectsShow", m_pChkEffectsShow);
	DDX_Control("Chk_ExcludeHost", m_pChkExcludeHost);
	DDX_Control("kwi_target_simplemodel", m_pChkTargetSimpleModel);
	
	// Labels
	DDX_Control("mx_modelLimitValue", m_pTxtModelLimitValue);
	DDX_Control("mx_mateLimitValue", m_pTxtMateLimitValue);
	DDX_Control("mx_enemyLimitValue", m_pTxtEnemyLimitValue);
	DDX_Control("mx_effectsDistenceValue", m_pTxtEffectsDistanceValue);
	DDX_Control("Txt_Dist", m_pTxtDist);
	DDX_Control("edi_dist", m_pEditDist);
	
	// Old checkboxes (for compatibility)
	m_pChk_Player = (PAUICHECKBOX)GetDlgItem("Chk_Player");
	m_pChk_Surround = (PAUICHECKBOX)GetDlgItem("Chk_Surround");
	m_pChk_Sight = (PAUICHECKBOX)GetDlgItem("Chk_Sight");
	
	return CDlgOptimizeBase::OnInitDialog();
}

void CDlgOptimizeMem::DoDataExchange(bool bSave)
{
	CDlgOptimizeBase::DoDataExchange(bSave);

	DDX_CheckBox(bSave, "Chk_OptimizeMem", m_bAutoOptimize);
	DDX_RadioButton(bSave, 0, m_iMemState);
}

void CDlgOptimizeMem::OnShowDialog()
{
	// Load settings from config
	m_setting = g_pGame->GetConfigs()->GetOptimizeSettings();
	m_bAutoOptimize = m_setting.bAutoSimplify;
	m_iMemState = m_setting.iSimplifyMode;
	
	// Check if loaded settings match Normal preset
	bool bMatchesNormal = (m_setting.nModelLimit == 210 && m_setting.nMateLimit == 100 && 
		m_setting.nEnemyLimit == 100 && m_setting.nSpEffect == 0 && 
		m_setting.nEffectsDistance == 100 && m_setting.nEffectsType == 0 && 
		m_setting.nNameDistance == 100 && !m_setting.bPeopleLimit && 
		!m_setting.bModelLimit && !m_setting.bPeopleLimitByNear && 
		!m_setting.bSpEffect && !m_setting.bEffectsDistance && 
		!m_setting.bEffectsShow && !m_setting.bExcludeHost && 
		!m_setting.bTargetSimpleModel);
	
	// Check if loaded settings match Quick preset
	bool bMatchesQuick = (m_setting.nModelLimit == 50 && m_setting.nMateLimit == 50 && 
		m_setting.nEnemyLimit == 50 && m_setting.nSpEffect == 3 && 
		m_setting.nEffectsDistance == 50 && m_setting.nEffectsType == 3 && 
		m_setting.nNameDistance == 50 && m_setting.bPeopleLimit && 
		m_setting.bModelLimit && m_setting.bPeopleLimitByNear && 
		m_setting.bSpEffect && m_setting.bEffectsDistance && 
		m_setting.bEffectsShow && m_setting.bExcludeHost && 
		m_setting.bTargetSimpleModel);
	
	// If settings don't match any preset and nConfigMode is 0 or 1, mark as custom
	if (!bMatchesNormal && !bMatchesQuick && (m_setting.nConfigMode == 0 || m_setting.nConfigMode == 1))
		m_setting.nConfigMode = 2; // Custom mode
	
	// If settings match a preset, update nConfigMode accordingly
	if (bMatchesNormal && m_setting.nConfigMode != 0)
		m_setting.nConfigMode = 0; // Normal preset
	else if (bMatchesQuick && m_setting.nConfigMode != 1)
		m_setting.nConfigMode = 1; // Quick preset
	
	UpdateMemView();
	UpdateSettingsView();
	OnCommand_OptimizeMem(NULL);
}

void CDlgOptimizeMem::UpdateMemView()
{
	CECMemSimplify* pMemSimplify = g_pGame->GetGameRun()->GetMemSimplify();
	m_bAutoOptimize = pMemSimplify->IsAutoOptimize();
	m_iMemState = pMemSimplify->GetGlobalMemUsage();

	UpdateData(false);
}

void CDlgOptimizeMem::OnCommand_Confirm(const char *szCommand)
{
	ApplySettings();
	Show(false);
}

void CDlgOptimizeMem::OnCommand_Apply(const char *szCommand)
{
	ApplySettings();
}

void CDlgOptimizeMem::OnCommand_OptimizeMem(const char* szCommand)
{
	UpdateData();
	GetDlgItem("Rdo_MemState1")->Enable(!m_bAutoOptimize);
	GetDlgItem("Rdo_MemState2")->Enable(!m_bAutoOptimize);
	GetDlgItem("Rdo_MemState3")->Enable(!m_bAutoOptimize);
	GetDlgItem("Rdo_MemState4")->Enable(!m_bAutoOptimize);
	if (GetDlgItem("Rdo_MemState5"))
		GetDlgItem("Rdo_MemState5")->Enable(!m_bAutoOptimize);
}

void CDlgOptimizeMem::UpdateSettingsView()
{
	// Update radio buttons
	if (m_pRdoNormal) m_pRdoNormal->Check(m_setting.nConfigMode == 0);
	if (m_pRdoQuick) m_pRdoQuick->Check(m_setting.nConfigMode == 1);
	
	// Update sliders
	if (m_pSliderModelLimit) m_pSliderModelLimit->SetLevel(m_setting.nModelLimit);
	// Slider has 0-89 levels, but setting is 0-100%, so convert: level = (percent / 100) * 89
	if (m_pSliderMateLimit) m_pSliderMateLimit->SetLevel((BYTE)((m_setting.nMateLimit / 100.0f) * 89.0f + 0.5f));
	if (m_pSliderEnemyLimit) m_pSliderEnemyLimit->SetLevel((BYTE)((m_setting.nEnemyLimit / 100.0f) * 89.0f + 0.5f));
	if (m_pSliderSpEffect) m_pSliderSpEffect->SetLevel(m_setting.nSpEffect);
	if (m_pSliderEffectsDistance) m_pSliderEffectsDistance->SetLevel(m_setting.nEffectsDistance);
	if (m_pSliderEffectsType) m_pSliderEffectsType->SetLevel(m_setting.nEffectsType);
	if (m_pSliderNameDistance) m_pSliderNameDistance->SetLevel(m_setting.nNameDistance);
	
	// Update checkboxes
	if (m_pChkPeopleLimit) m_pChkPeopleLimit->Check(m_setting.bPeopleLimit);
	if (m_pChkModelLimit) m_pChkModelLimit->Check(m_setting.bModelLimit);
	if (m_pChkPeopleLimitByNear) m_pChkPeopleLimitByNear->Check(m_setting.bPeopleLimitByNear);
	if (m_pChkSpEffect) m_pChkSpEffect->Check(m_setting.bSpEffect);
	if (m_pChkEffectsDistance) m_pChkEffectsDistance->Check(m_setting.bEffectsDistance);
	if (m_pChkEffectsShow) m_pChkEffectsShow->Check(m_setting.bEffectsShow);
	if (m_pChkExcludeHost) m_pChkExcludeHost->Check(m_setting.bExcludeHost);
	if (m_pChkTargetSimpleModel) m_pChkTargetSimpleModel->Check(m_setting.bTargetSimpleModel);
	
	// Update labels
	UpdateValueLabels();
}

void CDlgOptimizeMem::UpdateValueLabels()
{
	ACString str;
	
	if (m_pTxtModelLimitValue)
	{
		str.Format(L"%d §Þ", (int)m_setting.nModelLimit); // Display value in meters
		m_pTxtModelLimitValue->SetText(str);
	}
	
	if (m_pTxtMateLimitValue)
	{
		// Display as percentage (0-100%), slider has 0-89 levels, so max level (89) = 100%
		str.Format(L"%d%%", (int)m_setting.nMateLimit);
		m_pTxtMateLimitValue->SetText(str);
	}
	
	if (m_pTxtEnemyLimitValue)
	{
		// Display as percentage (0-100%), slider has 0-89 levels, so max level (89) = 100%
		str.Format(L"%d%%", (int)m_setting.nEnemyLimit);
		m_pTxtEnemyLimitValue->SetText(str);
	}
	
	if (m_pTxtEffectsDistanceValue)
	{
		str.Format(L"%d", (int)m_setting.nEffectsDistance);
		m_pTxtEffectsDistanceValue->SetText(str);
	}
	
	if (m_pTxtDist || m_pEditDist)
	{
		str.Format(L"%d §Þ", (int)m_setting.nNameDistance);
		if (m_pTxtDist) m_pTxtDist->SetText(str);
		if (m_pEditDist) m_pEditDist->SetText(str);
	}
}

void CDlgOptimizeMem::ApplySettings()
{
	UpdateData();
	
	// Save from UI to settings
	// Determine config mode based on radio button selection
	// If neither preset is selected, it means user manually changed settings (custom mode)
	if (m_pRdoNormal && m_pRdoNormal->IsChecked())
		m_setting.nConfigMode = 0;
	else if (m_pRdoQuick && m_pRdoQuick->IsChecked())
		m_setting.nConfigMode = 1;
	else
		m_setting.nConfigMode = 2; // Custom mode (user manually changed settings)
	
	if (m_pSliderModelLimit) m_setting.nModelLimit = (BYTE)m_pSliderModelLimit->GetLevel();
	// Slider has 0-89 levels, but setting is 0-100%, so convert: percent = (level / 89) * 100
	if (m_pSliderMateLimit) m_setting.nMateLimit = (BYTE)((m_pSliderMateLimit->GetLevel() / 89.0f) * 100.0f + 0.5f);
	if (m_pSliderEnemyLimit) m_setting.nEnemyLimit = (BYTE)((m_pSliderEnemyLimit->GetLevel() / 89.0f) * 100.0f + 0.5f);
	if (m_pSliderSpEffect) m_setting.nSpEffect = (BYTE)m_pSliderSpEffect->GetLevel();
	if (m_pSliderEffectsDistance) m_setting.nEffectsDistance = (BYTE)m_pSliderEffectsDistance->GetLevel();
	if (m_pSliderEffectsType) m_setting.nEffectsType = (BYTE)m_pSliderEffectsType->GetLevel();
	if (m_pSliderNameDistance) m_setting.nNameDistance = (BYTE)m_pSliderNameDistance->GetLevel();
	
	if (m_pChkPeopleLimit) m_setting.bPeopleLimit = m_pChkPeopleLimit->IsChecked();
	if (m_pChkModelLimit) m_setting.bModelLimit = m_pChkModelLimit->IsChecked();
	if (m_pChkPeopleLimitByNear) m_setting.bPeopleLimitByNear = m_pChkPeopleLimitByNear->IsChecked();
	if (m_pChkSpEffect) m_setting.bSpEffect = m_pChkSpEffect->IsChecked();
	if (m_pChkEffectsDistance) m_setting.bEffectsDistance = m_pChkEffectsDistance->IsChecked();
	if (m_pChkEffectsShow) m_setting.bEffectsShow = m_pChkEffectsShow->IsChecked();
	if (m_pChkExcludeHost) m_setting.bExcludeHost = m_pChkExcludeHost->IsChecked();
	if (m_pChkTargetSimpleModel) m_setting.bTargetSimpleModel = m_pChkTargetSimpleModel->IsChecked();
	
	// Save auto optimize and simplify mode
	m_setting.bAutoSimplify = m_bAutoOptimize;
	m_setting.iSimplifyMode = m_iMemState;
	
	// Apply to game
	CECMemSimplify* pMemSimplify = g_pGame->GetGameRun()->GetMemSimplify();
	if (pMemSimplify)
	{
		pMemSimplify->SetAutoOptimize(m_bAutoOptimize);
		if (!m_bAutoOptimize)
			pMemSimplify->TransferUsageAll(m_iMemState);
	}
	
	// Save to config
	g_pGame->GetConfigs()->SetModelOptimize(m_bAutoOptimize, m_iMemState);
	g_pGame->GetConfigs()->SetOptimizeSettings(m_setting);
}

void CDlgOptimizeMem::OnCommand_Default(const char *szCommand)
{
	m_setting.Reset();
	m_bAutoOptimize = m_setting.bAutoSimplify; // Reset old fields too
	m_iMemState = m_setting.iSimplifyMode;
	UpdateMemView(); // Update old controls
	UpdateSettingsView(); // Update new controls
	ApplySettings(); // Apply default settings to game
}

void CDlgOptimizeMem::OnCommand_ModeChanged(const char *szCommand)
{
	if (m_pRdoNormal && m_pRdoNormal->IsChecked())
	{
		m_setting.nConfigMode = 0; // Normal config
		// Apply default values for normal config
		m_setting.nModelLimit = 210; // Max distance in meters
		m_setting.nMateLimit = 100;
		m_setting.nEnemyLimit = 100;
		m_setting.nSpEffect = 0;
		m_setting.nEffectsDistance = 100;
		m_setting.nEffectsType = 0;
		m_setting.nNameDistance = 100;
		m_setting.bPeopleLimit = false;
		m_setting.bModelLimit = false;
		m_setting.bPeopleLimitByNear = false;
		m_setting.bSpEffect = false;
		m_setting.bEffectsDistance = false;
		m_setting.bEffectsShow = false;
		m_setting.bExcludeHost = false;
		m_setting.bTargetSimpleModel = false;
	}
	else if (m_pRdoQuick && m_pRdoQuick->IsChecked())
	{
		m_setting.nConfigMode = 1; // Quick config
		// Apply optimized values for quick config
		m_setting.nModelLimit = 50;
		m_setting.nMateLimit = 50;
		m_setting.nEnemyLimit = 50;
		m_setting.nSpEffect = 3;
		m_setting.nEffectsDistance = 50;
		m_setting.nEffectsType = 3;
		m_setting.nNameDistance = 50;
		m_setting.bPeopleLimit = true;
		m_setting.bModelLimit = true;
		m_setting.bPeopleLimitByNear = true;
		m_setting.bSpEffect = true;
		m_setting.bEffectsDistance = true;
		m_setting.bEffectsShow = true;
		m_setting.bExcludeHost = true;
		m_setting.bTargetSimpleModel = true;
	}
	
	// Update UI to reflect new settings
	UpdateSettingsView();
	
	// Apply and save settings immediately when mode changes
	// This ensures the preset values are saved to config
	ApplySettings();
}

void CDlgOptimizeMem::OnCommand_ModelLimit(const char *szCommand)
{
	if (m_pSliderModelLimit)
	{
		m_setting.nModelLimit = (BYTE)m_pSliderModelLimit->GetLevel();
		// User manually changed setting, mark as custom
		if (m_setting.nConfigMode != 2)
			m_setting.nConfigMode = 2;
		// Uncheck preset radio buttons when manually changing settings
		if (m_pRdoNormal) m_pRdoNormal->Check(false);
		if (m_pRdoQuick) m_pRdoQuick->Check(false);
		UpdateValueLabels();
	}
}

void CDlgOptimizeMem::OnCommand_MateLimit(const char *szCommand)
{
	if (m_pSliderMateLimit)
	{
		// Slider has 0-89 levels, but setting is 0-100%, so convert: percent = (level / 89) * 100
		m_setting.nMateLimit = (BYTE)((m_pSliderMateLimit->GetLevel() / 89.0f) * 100.0f + 0.5f);
		// User manually changed setting, mark as custom
		if (m_setting.nConfigMode != 2)
			m_setting.nConfigMode = 2;
		// Uncheck preset radio buttons when manually changing settings
		if (m_pRdoNormal) m_pRdoNormal->Check(false);
		if (m_pRdoQuick) m_pRdoQuick->Check(false);
		UpdateValueLabels();
	}
}

void CDlgOptimizeMem::OnCommand_EnemyLimit(const char *szCommand)
{
	if (m_pSliderEnemyLimit)
	{
		// Slider has 0-89 levels, but setting is 0-100%, so convert: percent = (level / 89) * 100
		m_setting.nEnemyLimit = (BYTE)((m_pSliderEnemyLimit->GetLevel() / 89.0f) * 100.0f + 0.5f);
		// User manually changed setting, mark as custom
		if (m_setting.nConfigMode != 2)
			m_setting.nConfigMode = 2;
		// Uncheck preset radio buttons when manually changing settings
		if (m_pRdoNormal) m_pRdoNormal->Check(false);
		if (m_pRdoQuick) m_pRdoQuick->Check(false);
		UpdateValueLabels();
	}
}

void CDlgOptimizeMem::OnCommand_SpEffect(const char *szCommand)
{
	if (m_pSliderSpEffect)
	{
		m_setting.nSpEffect = (BYTE)m_pSliderSpEffect->GetLevel();
		// User manually changed setting, mark as custom
		if (m_setting.nConfigMode != 2)
			m_setting.nConfigMode = 2;
		// Uncheck preset radio buttons when manually changing settings
		if (m_pRdoNormal) m_pRdoNormal->Check(false);
		if (m_pRdoQuick) m_pRdoQuick->Check(false);
	}
}

void CDlgOptimizeMem::OnCommand_EffectsDistance(const char *szCommand)
{
	if (m_pSliderEffectsDistance)
	{
		m_setting.nEffectsDistance = (BYTE)m_pSliderEffectsDistance->GetLevel();
		// User manually changed setting, mark as custom
		if (m_setting.nConfigMode != 2)
			m_setting.nConfigMode = 2;
		// Uncheck preset radio buttons when manually changing settings
		if (m_pRdoNormal) m_pRdoNormal->Check(false);
		if (m_pRdoQuick) m_pRdoQuick->Check(false);
		UpdateValueLabels();
	}
}

void CDlgOptimizeMem::OnCommand_EffectsType(const char *szCommand)
{
	if (m_pSliderEffectsType)
	{
		m_setting.nEffectsType = (BYTE)m_pSliderEffectsType->GetLevel();
		// User manually changed setting, mark as custom
		if (m_setting.nConfigMode != 2)
			m_setting.nConfigMode = 2;
		// Uncheck preset radio buttons when manually changing settings
		if (m_pRdoNormal) m_pRdoNormal->Check(false);
		if (m_pRdoQuick) m_pRdoQuick->Check(false);
	}
}

void CDlgOptimizeMem::OnCommand_SldDist(const char *szCommand)
{
	if (m_pSliderNameDistance)
	{
		m_setting.nNameDistance = (BYTE)m_pSliderNameDistance->GetLevel();
		// User manually changed setting, mark as custom
		if (m_setting.nConfigMode != 2)
			m_setting.nConfigMode = 2;
		// Uncheck preset radio buttons when manually changing settings
		if (m_pRdoNormal) m_pRdoNormal->Check(false);
		if (m_pRdoQuick) m_pRdoQuick->Check(false);
		UpdateValueLabels();
	}
}

void CDlgOptimizeMem::OnCommand_AddDist(const char *szCommand)
{
	if (m_pSliderNameDistance && m_setting.nNameDistance < 210)
	{
		m_setting.nNameDistance++;
		m_pSliderNameDistance->SetLevel(m_setting.nNameDistance);
		UpdateValueLabels();
	}
}

void CDlgOptimizeMem::OnCommand_DecDist(const char *szCommand)
{
	if (m_pSliderNameDistance && m_setting.nNameDistance > 0)
	{
		m_setting.nNameDistance--;
		m_pSliderNameDistance->SetLevel(m_setting.nNameDistance);
		// User manually changed setting, mark as custom
		if (m_setting.nConfigMode != 2)
			m_setting.nConfigMode = 2;
		// Uncheck preset radio buttons when manually changing settings
		if (m_pRdoNormal) m_pRdoNormal->Check(false);
		if (m_pRdoQuick) m_pRdoQuick->Check(false);
		UpdateValueLabels();
	}
}

void CDlgOptimizeMem::OnCommand_TargetSimpleModel(const char *szCommand)
{
	if (m_pChkTargetSimpleModel)
		m_setting.bTargetSimpleModel = m_pChkTargetSimpleModel->IsChecked();
}
