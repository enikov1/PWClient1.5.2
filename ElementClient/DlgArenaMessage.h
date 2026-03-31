// Filename: DlgArenaMessage.h
// Arena of Aurora - Message dialog (battle confirmation)

#pragma once

#include "DlgBase.h"
#include "AUIStillImageButton.h"
#include "AUILabel.h"

class CDlgArenaMessage : public CDlgBase
{
	AUI_DECLARE_COMMAND_MAP()

public:
	CDlgArenaMessage();
	virtual ~CDlgArenaMessage();

	void SetCountdownTime(int time_rest);

protected:
	virtual bool OnInitDialog();
	virtual void OnShowDialog();
	virtual void OnHideDialog();
	virtual void OnTick();

	PAUILABEL m_pTxt_time;
	int m_nTotalTime;
	int m_nEndTime;

	virtual void OnCommand_Enter(const char* szCommand);
	virtual void OnCommand_Exit(const char* szCommand);
};

