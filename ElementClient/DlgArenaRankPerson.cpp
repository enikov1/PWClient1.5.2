// Filename: DlgArenaRankPerson.cpp
// Arena of Aurora - Personal Ranking dialog implementation

#include "DlgArenaRankPerson.h"
#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EC_GameSession.h"
#include "EC_Global.h"
#include "EC_GameUIMan.h"
#include "EC_HostPlayer.h"
#include "AUILabel.h"
#include "AUIListBox.h"
#include "AUICommon.h"
#include "EC_StringTab.h"
#include "EC_Utility.h"
#include "EC_ArenaRankConfig.h"

AUI_BEGIN_COMMAND_MAP(CDlgArenaRankPerson, CDlgBase)
AUI_ON_COMMAND("arenarank_team", OnCommand_BtnTeam)
AUI_ON_COMMAND("arenarank_person", OnCommand_BtnPerson)
AUI_ON_COMMAND("IDCANCEL", OnCommand_Close)
AUI_ON_COMMAND("Rdo_0", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_1", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_2", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_3", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_4", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_5", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_6", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_7", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_8", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_9", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_10", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_11", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_12", OnCommand_RadioClass)
AUI_ON_COMMAND("Rdo_13", OnCommand_RadioClass)
AUI_END_COMMAND_MAP()

CDlgArenaRankPerson::CDlgArenaRankPerson()
{
	m_pList = NULL;
	m_pBtnTeam = NULL;
	m_pBtnPerson = NULL;
	m_pBtnClose = NULL;
	m_nRankMode = 0; // Personal by default
	m_nBattleMode = 1; // 3x3 by default
	m_nClassFilter = -1; // All classes
}

CDlgArenaRankPerson::~CDlgArenaRankPerson()
{
}

bool CDlgArenaRankPerson::OnInitDialog()
{
	if (!CDlgBase::OnInitDialog())
		return false;

	DDX_Control("List", m_pList);
	DDX_Control("Btn_Team", m_pBtnTeam);
	DDX_Control("Btn_Person", m_pBtnPerson);
	DDX_Control("Btn_Close", m_pBtnClose);

	return true;
}

void CDlgArenaRankPerson::OnShowDialog()
{
	// Initialize button states
	if (m_pBtnTeam) m_pBtnTeam->Enable(true);
	if (m_pBtnPerson) 
	{
		m_pBtnPerson->Enable(false); // Personal mode is default
	}
	
	// Set default mode
	m_nRankMode = 0; // Personal
	m_nBattleMode = 1; // 3x3 (ButtonID 1 = Rdo33)
	
	// Set default radio buttons state
	CheckRadioButton(0, 1); // GroupID 0, ButtonID 1 (Rdo33 - 3x3)
	CheckRadioButton(1, 14); // GroupID 1, ButtonID 14 (Btn_All)
	
	// Clear previous data
	m_aPlayerList.GetVector().clear();
	m_aTeamList.GetVector().clear();
	if (m_pList) m_pList->ResetContent();
	
	// Request data when dialog is shown (delay to ensure dialog is fully initialized)
	RequestPlayerRanking();
}

void CDlgArenaRankPerson::OnHideDialog()
{
}

void CDlgArenaRankPerson::OnTick()
{
	// Check if battle mode radio button has changed
	int nIndex = -1;
	DDX_RadioButton(true, 0, nIndex);
	
	if (nIndex >= 0)
	{
		int newBattleMode = (nIndex == 1) ? 1 : 2; // 1 = 3x3, 2 = 6x6
		
		if (newBattleMode != m_nBattleMode)
		{
			m_nBattleMode = newBattleMode;
			// Re-request data with new mode
			if (m_nRankMode == 0)
				RequestPlayerRanking();
			else
				RequestTeamRanking();
		}
	}
}

void CDlgArenaRankPerson::RequestPlayerRanking()
{
	// Read current radio button selection (battle mode: 3x3 or 6x6)
	int nIndex = -1;
	DDX_RadioButton(true, 0, nIndex);
	
	if (nIndex == 1) // Rdo33 - ButtonID 1
		m_nBattleMode = 1; // 3x3
	else if (nIndex == 2) // Rdo66 - ButtonID 2
		m_nBattleMode = 2; // 6x6
	else
		m_nBattleMode = 1; // Default to 3x3
	
	// TODO: Implement sending request to server
	// For now, this is a placeholder
	// The actual implementation should send EC_ArenaPlayerTopListQuery protocol
	// via SendNetData or through service provider
	
	if (!g_pGame || !g_pGame->GetGameSession())
		return;

	// Get player roleid
	int roleid = g_pGame->GetGameSession()->GetCharacterID();
	if (roleid <= 0)
		return;

	// cmd_mode: 0 = individual, 1 = command mode
	// For personal ranking, use cmd_mode = 0
	int cmd_mode = 0;
	
	// Send protocol to server
	g_pGame->GetGameSession()->c2s_SendArenaPlayerTopListQuery(roleid, cmd_mode);
}

void CDlgArenaRankPerson::RequestTeamRanking()
{
	// Read current radio button selection (battle mode: 3x3 or 6x6)
	int nIndex = -1;
	DDX_RadioButton(true, 0, nIndex);
	
	if (nIndex == 1) // Rdo33 - ButtonID 1
		m_nBattleMode = 1; // 3x3
	else if (nIndex == 2) // Rdo66 - ButtonID 2
		m_nBattleMode = 2; // 6x6
	else
		m_nBattleMode = 1; // Default to 3x3
	
	if (!g_pGame || !g_pGame->GetGameSession())
		return;

	// Get player roleid
	int roleid = g_pGame->GetGameSession()->GetCharacterID();
	if (roleid <= 0)
		return;

	// team_type: 1 = 3x3, 3 = 6x6 (server expects 1 or 3)
	int team_type = (m_nBattleMode == 1) ? 1 : 3;
	
	// Send protocol to server
	g_pGame->GetGameSession()->c2s_SendArenaTeamTopListQuery(roleid, team_type);
}

void CDlgArenaRankPerson::UpdateRankList()
{
	if (!m_pList)
		return;

	m_pList->ResetContent();

	if (m_nRankMode == 0)
	{
		// Personal ranking mode
		const std::vector<GNET::EC_ArenaPlayerTopList>& vec = m_aPlayerList.GetVector();
		
		int nRank = 1;
		for (size_t i = 0; i < vec.size(); ++i)
		{
			const GNET::EC_ArenaPlayerTopList& player = vec[i];
			
			// Filter by class if needed
			if (m_nClassFilter >= 0 && (int)player.cls != m_nClassFilter)
				continue;

			ACString strLine;
			ACString strRank, strName, strServer, strClass, strScore, strWin, strRankLevel;
			
			strRank.Format(TEXT("%d"), nRank++);
			strName = ACString((const ACHAR*)player.name.begin(), player.name.size() / sizeof(ACHAR));
			
			// Get class name
			if (g_pGame && g_pGame->GetGameRun())
				strClass = g_pGame->GetGameRun()->GetProfName((int)player.cls);
			else
				strClass.Format(TEXT("%d"), (int)player.cls);
			
			strScore.Format(TEXT("%u"), player.score);
			strWin.Format(TEXT("%u"), player.win_count);
			
			// Calculate rank level from score using config
			int nRankLevel = EC_ArenaRankConfig::GetInstance()->GetRankByScore(player.score);
			strRankLevel.Format(TEXT("%d"), nRankLevel);

			// Format: Rank | Name | Server | Class | Win | Score | RankLevel
			strLine.Format(TEXT("%s\t%s\t%s\t%s\t%s\t%s\t%s"),
				(const ACHAR*)strRank, (const ACHAR*)strName, TEXT("-"), (const ACHAR*)strClass,
				(const ACHAR*)strWin, (const ACHAR*)strScore, (const ACHAR*)strRankLevel);
			
			m_pList->AddString(strLine);
		}
	}
	else
	{
		// Team ranking mode
		const std::vector<GNET::EC_ArenaTeamTopList>& vec = m_aTeamList.GetVector();
		
		int nRank = 1;
		for (size_t i = 0; i < vec.size(); ++i)
		{
			const GNET::EC_ArenaTeamTopList& team = vec[i];
			
			ACString strLine;
			ACString strRank, strTeamName, strCaptain, strScore, strWin;
			
			strRank.Format(TEXT("%d"), nRank++);
			strTeamName = ACString((const ACHAR*)team.name.begin(), team.name.size() / sizeof(ACHAR));
			strScore.Format(TEXT("%u"), team.team_score);
			strWin.Format(TEXT("%u"), team.win_count);
			
			// Format: Rank | Team Name | Captain | Score | Win
			strLine.Format(TEXT("%s\t%s\t%s\t%s\t%s"),
				(const ACHAR*)strRank, (const ACHAR*)strTeamName, TEXT("-"), (const ACHAR*)strScore, (const ACHAR*)strWin);
			
			m_pList->AddString(strLine);
		}
	}
}

void CDlgArenaRankPerson::FilterByClass(int clsFilter)
{
	m_nClassFilter = clsFilter;
	UpdateRankList();
}

void CDlgArenaRankPerson::OnCommand_BtnTeam(const char* szCommand)
{
	m_nRankMode = 1; // Team mode
	if (m_pBtnTeam) m_pBtnTeam->Enable(false);
	if (m_pBtnPerson) m_pBtnPerson->Enable(true);
	
	// Re-request with current radio button selection
	RequestTeamRanking();
}

void CDlgArenaRankPerson::OnCommand_BtnPerson(const char* szCommand)
{
	m_nRankMode = 0; // Personal mode
	if (m_pBtnTeam) m_pBtnTeam->Enable(true);
	if (m_pBtnPerson) m_pBtnPerson->Enable(false);
	
	// Re-request with current radio button selection
	RequestPlayerRanking();
}

void CDlgArenaRankPerson::OnCommand_Close(const char* szCommand)
{
	Show(false);
}

void CDlgArenaRankPerson::OnCommand_RadioClass(const char* szCommand)
{
	int nIndex = -1;
	DDX_RadioButton(true, 1, nIndex);
	
	if (nIndex == 14) // Btn_All
		m_nClassFilter = -1;
	else
		m_nClassFilter = nIndex - 1; // Adjust to 0-13 range
	
	// Just update filter, don't re-request data
	if (m_nRankMode == 0)
		UpdateRankList();
}

void CDlgArenaRankPerson::OnArenaPlayerTopListReceived(EC_ArenaPlayerTopListQuery_Re* pNotify)
{
	if (!pNotify)
		return;

	if (pNotify->retcode != 0)
		return;

	m_aPlayerList = pNotify->members;
	UpdateRankList();
}

void CDlgArenaRankPerson::OnArenaTeamTopListReceived(EC_ArenaTeamTopListQuery_Re* pNotify)
{
	if (!pNotify)
		return;

	if (pNotify->retcode != 0)
		return;

	m_aTeamList = pNotify->teams;
	UpdateRankList();
}

