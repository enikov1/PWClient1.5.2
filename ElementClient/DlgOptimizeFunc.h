// Filename	: DlgOptimizeFunc.h
// Creator	: zhangyitian
// Date		: 2013/9/27

#ifndef _ELEMENTCLIENT_DLGOPTIMIZEFUNC_H_
#define _ELEMENTCLIENT_DLGOPTIMIZEFUNC_H_

#include "DlgOptimizeBase.h"
#include "EC_Configs.h"

class AUICheckBox;

class CDlgOptimizeFunc : public CDlgOptimizeBase {

	AUI_DECLARE_COMMAND_MAP();

protected:
	virtual bool OnInitDialog();
	virtual void OnShowDialog();
	
    void UpdateFunctionView();

private:
	AUICheckBox* m_pChkLowHPGfx;
	AUICheckBox* m_pChkEquipCompare;
	AUICheckBox* m_pChkPetBuff;
	AUICheckBox* m_pChkLockQBar;
	AUICheckBox* m_pChkHideIceThunderBall;  // 隐藏月仙冰雷球界面
	AUICheckBox* m_pChkHideTargetOfTarget;	// 关闭目标的目标
	AUICheckBox* m_pChkHideElf;				// Hide elf/fairy display
	AUICheckBox* m_pChkHideQuest;			// Hide quest info on monsters
	AUICheckBox* m_pChkBuff;				// Normal buff display
	AUICheckBox* m_pChkUnfreeze;			// Unfreeze window
	AUICheckBox* m_pChkOffFly;				// Disable fly model
	AUICheckBox* m_pChkOffHair;				// Disable hair rendering
	AUICheckBox* m_pChkNoDelay;				// Disable skill delay
	AUICheckBox* m_pChkAutoComplete;		// Auto-complete skills
	AUICheckBox* m_pChkOffUnselect;			// Don't cancel target on Esc
	AUICheckBox* m_pChkCDTimer;				// Show skill cooldown timer
	AUICheckBox* m_pChkOffWeapon;			// Disable fashion weapon
	AUICheckBox* m_pChkFastFly;				// Remove fly mount animation
	AUICheckBox* m_pChkOffBooth;			// Disable trading booth display
	AUICheckBox* m_pChkHP;					// Show target HP (kwi_hp, uses UI settings)
	AUICheckBox* m_pChkTeamHP;				// Show team HP (kwi_teamhp, uses UI settings)

	EC_GAME_SETTING m_setting;

public:
	CDlgOptimizeFunc();
	
    void OnCommand_Confirm(const char *szCommand);
	void OnCommand_SelectAll(const char *szCommand);
    void OnCommand_Clear(const char *szCommand);
	void OnCommand_RadiusName(const char * szCommand);
	void OnCommand_RadiusModel(const char * szCommand);
	void OnCommand_Unfreeze(const char *szCommand);
	void OnCommand_OffFly(const char *szCommand);
	void OnCommand_OffHair(const char *szCommand);
	void OnCommand_NoDelay(const char *szCommand);
	void OnCommand_AutoComplete(const char *szCommand);
	void OnCommand_OffUnselect(const char *szCommand);
	void OnCommand_CDTimer(const char *szCommand);
	void OnCommand_OffWeapon(const char *szCommand);
	void OnCommand_FastFly(const char *szCommand);
	void OnCommand_OffBooth(const char *szCommand);
	void OnCommand_HP(const char *szCommand);
	void OnCommand_TeamHP(const char *szCommand);

	void SetLockQBar(bool bLock);
};

#endif

