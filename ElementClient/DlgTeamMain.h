// File		: DlgTeamMain.h
// Creator	: Xiao Zhou
// Date		: 2005/8/17

#pragma once

#include "DlgBase.h"
#include "AUILabel.h"

class CDlgTeamMate;
class CDlgTeamMain : public CDlgBase  
{
	friend class CDlgWorldMap;

	AUI_DECLARE_COMMAND_MAP()

public:
	CDlgTeamMain();
	virtual ~CDlgTeamMain();

	void OnCommandCANCAL(const char *szCommand);
	void OnCommandMinimize(const char *szCommand);
	void OnCommandMinimizeBuff(const char *szCommand);
	void OnCommandArrow(const char *szCommand);
	void OnCommandGTTalk(const char *szCommand);
	void OnCommandReformSquad(const char *szCommand);

	bool UpdateTeamInfo();
	bool IsShowTeamArrow() { return m_bShowTeamArrow; }

	CDlgTeamMate* GetTeamMateDlg(int index);
	void ProcessGTEnter();
	void ProcessReformSquad();  // Process reform squad invitations

protected:
	virtual bool OnInitDialog();
	PAUILABEL m_pTxt_hpteam; //new
private:
	bool m_bShowTeamArrow;
	bool m_bShowBuff;

	abase::vector<CDlgTeamMate*> m_pMates;
	
	// Variables for reform squad functionality
	abase::vector<int> m_aReformMemberList;  // List of member IDs to reinvite
	bool m_bReformInProgress;                // Flag indicating reform is in progress
	DWORD m_dwLastInviteTime;                // Time of last invite sent (for delay)
	int m_nCurrentInviteIndex;               // Current index in invite list

	enum
	{
		TEAM_STATUS_OPEN = 0,
		TEAM_STATUS_CLOSE
	};

	bool m_bGTWaiting;
};
