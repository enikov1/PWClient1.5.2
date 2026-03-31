// Filename: DlgArenaRankPerson.h
// Arena of Aurora - Personal Ranking dialog

#pragma once

#include "DlgBase.h"
#include "AUIStillImageButton.h"
#include "AUILabel.h"
#include "AUIListBox.h"
#include "AUIRadioButton.h"
#include "ec_arenaplayertoplistquery_re.hpp"
#include "ec_arenateamtoplistquery_re.hpp"

class CDlgArenaRankPerson : public CDlgBase
{
	AUI_DECLARE_COMMAND_MAP()

public:
	CDlgArenaRankPerson();
	virtual ~CDlgArenaRankPerson();

	void OnArenaPlayerTopListReceived(EC_ArenaPlayerTopListQuery_Re* pNotify);
	void OnArenaTeamTopListReceived(EC_ArenaTeamTopListQuery_Re* pNotify);

protected:
	virtual bool OnInitDialog();
	virtual void OnShowDialog();
	virtual void OnHideDialog();
	virtual void OnTick();

	void RequestPlayerRanking();
	void RequestTeamRanking();
	void UpdateRankList();
	void FilterByClass(int clsFilter);
	
	void OnCommand_BtnTeam(const char* szCommand);
	void OnCommand_BtnPerson(const char* szCommand);
	void OnCommand_Close(const char* szCommand);
	void OnCommand_RadioClass(const char* szCommand);

	AUIListBox* m_pList;
	PAUISTILLIMAGEBUTTON m_pBtnTeam;
	PAUISTILLIMAGEBUTTON m_pBtnPerson;
	PAUISTILLIMAGEBUTTON m_pBtnClose;

	// Mode: 0 = Personal, 1 = Team
	int m_nRankMode;
	// Battle mode: 1 = 3x3, 2 = 6x6
	int m_nBattleMode;
	// Class filter: -1 = All, 0-13 = specific class
	int m_nClassFilter;

	// Cached data
	GNET::EC_ArenaPlayerTopListVector m_aPlayerList;
	GNET::EC_ArenaTeamTopListVector m_aTeamList;
};

