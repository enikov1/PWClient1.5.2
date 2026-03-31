// Filename: DlgArenaQueue.h
// Arena of Aurora - Queue dialog

#pragma once

#include "DlgBase.h"
#include "AUIStillImageButton.h"
#include "AUILabel.h"
#include "AUIImagePicture.h"

class CDlgArenaQueue : public CDlgBase
{
public:
	CDlgArenaQueue();
	virtual ~CDlgArenaQueue();

	void SetupMatchmakingQueueFlag();
	void SetupArenaFlag();
	void SetQueueTime(int time_rest);

protected:
	virtual bool OnInitDialog();
	virtual void OnShowDialog();
	virtual void OnHideDialog();
	virtual void OnTick();

	int m_nTotalTime;
	int m_nEndTime;

	PAUILABEL m_pTxt_time;
	PAUISTILLIMAGEBUTTON m_pBtn_car;
	PAUISTILLIMAGEBUTTON m_pBtn_detail;
	PAUISTILLIMAGEBUTTON m_pBtn_GuildBattle;
	PAUISTILLIMAGEBUTTON m_pBtn_Divination;
	PAUIIMAGEPICTURE m_pGfx_1;
};

