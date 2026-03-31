// Filename	: DlgOptimizeBase.cpp
// Creator	: Xu Wenbin
// Date		: 2013/4/17

#include "DlgOptimizeBase.h"
#include "EC_Optimize.h"
#include "EC_Game.h"
#include "EC_GameUIMan.h"

#define new A_DEBUG_NEW

AUI_BEGIN_COMMAND_MAP(CDlgOptimizeBase, CDlgBase)
AUI_ON_COMMAND("Btn_SwitchMem", OnCommand_Switch)
AUI_ON_COMMAND("kwi_SwitchMem", OnCommand_Switch)
AUI_ON_COMMAND("Btn_SwitchGfx", OnCommand_Switch)
AUI_ON_COMMAND("Btn_SwitchFunction", OnCommand_Switch)
AUI_ON_COMMAND("kwi_SwitchFunction", OnCommand_Switch)
AUI_ON_COMMAND("kwi_SwitchUI", OnCommand_Switch)
AUI_ON_COMMAND("Btn_SwitchUI", OnCommand_Switch)
AUI_ON_COMMAND("Btn_OptimizeOS", OnCommand_OptimizeOS)
AUI_END_COMMAND_MAP()

CDlgOptimizeBase::CDlgOptimizeBase()
:m_pTxt_Memory(NULL)
,m_pTxt_FPS(NULL)
{
}

CDlgOptimizeBase::~CDlgOptimizeBase()
{
}

bool CDlgOptimizeBase::OnInitDialog()
{
	DDX_Control("Txt_Memory", m_pTxt_Memory);
	DDX_Control("Txt_FPS", m_pTxt_FPS);
    return true;
}

void CDlgOptimizeBase::OnTick()
{
    UpdateSysView();
}

void CDlgOptimizeBase::UpdateSysView()
{
    ACString str;
    SIZE_T mem(0);
    if (CECOptimize::Instance().GetVirtualSize(mem)){
		str = GetBaseUIMan()->GetFormatNumber(mem/1024);
    }else{
        str.Empty();
    }
	if (m_pTxt_Memory)
		m_pTxt_Memory->SetText(str);
    
    str.Format(_AL("%.1f"), GetGame()->GetAverageFrameRate());
	if (m_pTxt_FPS)
		m_pTxt_FPS->SetText(str);    
}

void CDlgOptimizeBase::OnCommand_Switch(const char *szCommand)
{
	PAUIDIALOG pDlgOptimizeMem = GetGameUIMan()->GetDialog("Win_OptimizeMem");
	PAUIDIALOG pDlgOptimizeFunc = GetGameUIMan()->GetDialog("Win_OptimizeFunc");
	PAUIDIALOG pDlgOptimizeUI = GetGameUIMan()->GetDialog("Win_OptimizeGfx"); // Use Win_OptimizeGfx XML for UI settings
	
	PAUIDIALOG pDlgCurrent = NULL;
	if (pDlgOptimizeMem && pDlgOptimizeMem->IsShow()){
		pDlgCurrent = pDlgOptimizeMem;
	} else if (pDlgOptimizeFunc && pDlgOptimizeFunc->IsShow()) {
		pDlgCurrent = pDlgOptimizeFunc;
	} else if (pDlgOptimizeUI && pDlgOptimizeUI->IsShow()) {
		pDlgCurrent = pDlgOptimizeUI;
	}
	if (!pDlgCurrent) return;

	PAUIDIALOG pDlgSwitchTo = NULL;
	AString strCommand = szCommand;
	if (strCommand == "Btn_SwitchMem" || strCommand == "kwi_SwitchMem"){
		pDlgSwitchTo = pDlgOptimizeMem;
	} else if (strCommand == "Btn_SwitchFunction" || strCommand == "kwi_SwitchFunction") {
		pDlgSwitchTo = pDlgOptimizeFunc;
	} else if (strCommand == "kwi_SwitchUI" || strCommand == "Btn_SwitchUI" || strCommand == "Btn_SwitchGfx") {
		// Use Win_OptimizeGfx XML for UI settings dialog
		// Btn_SwitchGfx also switches to UI dialog since CDlgOptimizeGfx was removed
		pDlgSwitchTo = GetGameUIMan()->GetDialog("Win_OptimizeGfx");
	}

	if (!pDlgSwitchTo) return;
	if (pDlgCurrent == pDlgSwitchTo) return;
	
	POINT pos = pDlgCurrent->GetPos();
	pDlgCurrent->OnCommand("IDCANCEL");

	pDlgSwitchTo->SetPosEx(pos.x, pos.y);	
	pDlgSwitchTo->Show(true);
	GetGameUIMan()->m_pDlgOptimizeCurrent = pDlgSwitchTo;
}

void CDlgOptimizeBase::OnCommand_OptimizeOS(const char *szCommand)
{
	if (PAUIDIALOG pDlgOptimizeOS = GetGameUIMan()->GetDialog("Win_OptimizeOS")){
		pDlgOptimizeOS->Show(!pDlgOptimizeOS->IsShow());
	}
}