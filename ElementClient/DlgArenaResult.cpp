// Filename: DlgArenaResult.cpp
// Arena of Aurora - Battle result dialog implementation

#include "DlgArenaResult.h"
#include "ArenaUtils.h"
#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EC_GameSession.h"
#include "EC_Global.h"
#include "EC_GameUIMan.h"
#include "EC_UIManager.h"
#include "EC_Resource.h"
#include "EC_HostPlayer.h"
#include "EC_Player.h"
#include "EC_Model.h"
#include "DlgArenaPoint.h"
#include "DlgArenaQueue.h"
#include "Network/EC_GPDataType.h"

AUI_BEGIN_COMMAND_MAP(CDlgArenaResult, CDlgBase)
AUI_ON_COMMAND("Btn_Departure", ExitArena)
AUI_ON_COMMAND("IDCANCEL", OnCommandCANCAL)
AUI_END_COMMAND_MAP()

AUI_BEGIN_EVENT_MAP(CDlgArenaResult, CDlgBase)
AUI_END_EVENT_MAP()

CDlgArenaResult::CDlgArenaResult()
{
	exitcount = 0;
	m_pBtn_Departure = NULL;
	m_pLst1 = NULL;
	m_pLst2 = NULL;
}

CDlgArenaResult::~CDlgArenaResult()
{
}

void CDlgArenaResult::OnCommandCANCAL(const char* szCommand)
{
}

bool CDlgArenaResult::OnInitDialog()
{
	DDX_Control("Btn_Departure", m_pBtn_Departure);
	DDX_Control("List_1", m_pLst1);
	DDX_Control("List_2", m_pLst2);
	if (m_pLst1) m_pLst1->ResetContent();
	if (m_pLst2) m_pLst2->ResetContent();

	PAUIOBJECT pObj;
	pObj = GetDlgItem("Img_B66"); if (pObj) pObj->Show(false);
	pObj = GetDlgItem("Img_R66"); if (pObj) pObj->Show(false);
	pObj = GetDlgItem("Img_EnemyB66"); if (pObj) pObj->Show(false);
	pObj = GetDlgItem("Img_EnemyR66"); if (pObj) pObj->Show(false);
	pObj = GetDlgItem("Img_B33"); if (pObj) pObj->Show(false);
	pObj = GetDlgItem("Img_R33"); if (pObj) pObj->Show(false);
	pObj = GetDlgItem("Img_EnemyR33"); if (pObj) pObj->Show(false);
	pObj = GetDlgItem("Img_EnemyB33"); if (pObj) pObj->Show(false);
	return true;
}

void CDlgArenaResult::OnShowDialog()
{
}

void CDlgArenaResult::OnHideDialog()
{
}

void CDlgArenaResult::OnTick()
{
	if (m_pBtn_Departure)
	{
		int decreasedcount = exitcount - GetGame()->GetServerGMTTime();
		ACString str;
		if (decreasedcount <= 0)
		{
			// Only clear and hide if countdown finished
			if (exitcount > 0)  // Make sure exitcount was set
			{
				ClearArenaBattleInfo();
				str.Format(GetStringFromTable(20011));
				m_pBtn_Departure->SetText(str);
				Show(false);
			}
		}
		else
		{
			str.Format(GetStringFromTable(20012), decreasedcount);
			m_pBtn_Departure->SetText(str);
		}
	}
}

void CDlgArenaResult::ExitArena(const char* szCommand)
{
	ClearArenaBattleInfo();
	g_pGame->GetGameSession()->c2s_SendCmdArenaExit();
	Show(false);
}

void CDlgArenaResult::ClearArenaBattleInfo()
{
	CECGameUIMan* pGameUI = g_pGame->GetGameRun()->GetUIManager()->GetInGameUIMan();
	static const char* szHeadHook = "HH_Head";

	CECHostPlayer::BATTLEINFO binfo = GetHostPlayer()->GetBattleInfo();
	binfo.nType = CECHostPlayer::BT_NONE;
	binfo.idBattle = 0;
	binfo.iEndTime = 0;
	GetHostPlayer()->RemoveGfx(res_GFXFile(RES_GFX_BLUEFLAGARENA), szHeadHook, PLAYERMODEL_TYPEALL);
	GetHostPlayer()->RemoveGfx(res_GFXFile(RES_GFX_REDFLAGARENA), szHeadHook, PLAYERMODEL_TYPEALL);

	GetHostPlayer()->SetBattleCamp(0);

	CDlgArenaPoint* dlg = dynamic_cast<CDlgArenaPoint*>(pGameUI->GetDialog("Win_ArenaPoint"));
	if (dlg)
	{
		dlg->Show(false);
		dlg->HideEnemiesDlg();
	}

	CDlgArenaQueue* dlg2 = dynamic_cast<CDlgArenaQueue*>(pGameUI->GetDialog("Win_ArenaQueue"));
	if (dlg2) dlg2->Show(false);
}

void CDlgArenaResult::SetResultInfo(void* pData)
{
	using namespace S2C;

	if (!pData || !m_pLst1 || !m_pLst2)
		return;

	cmd_arena_battle_result* pCmd = (cmd_arena_battle_result*)pData;

	// Set exit countdown timer (45 seconds after results shown)
	exitcount = GetGame()->GetServerGMTTime() + 45;

	// Validate array counts to prevent buffer overflows
	if (pCmd->red_count_players < 0 || pCmd->red_count_players > 6 ||
		pCmd->blue_count_players < 0 || pCmd->blue_count_players > 6)
	{
		// Invalid player counts, skip parsing
		return;
	}

	// Calculate expected packet size
	size_t fixedSize = sizeof(cmd_arena_battle_result);
	size_t redArraySize = pCmd->red_count_players * sizeof(arena_player_result);
	size_t blueArraySize = pCmd->blue_count_players * sizeof(arena_player_result);
	size_t expectedSize = fixedSize + redArraySize + blueArraySize;

	// Note: We can't validate the actual packet size here because we don't have it
	// The validation should be done in EC_GameDataPrtc.cpp before calling this function
	// But we can still validate array bounds during parsing

	m_pLst1->ResetContent();
	m_pLst2->ResetContent();

	// Determine battle mode from team counts
	int battleMode = (pCmd->red_count_players == 6 || pCmd->blue_count_players == 6) ? 1 : 0;

	// Show appropriate mode icons
	PAUIOBJECT pObj;
	if (battleMode == 0)
	{
		// 3x3 mode
		if (GetHostPlayer()->GetBattleCamp() == GP_BATTLE_CAMP_DEFENDER)
		{
			pObj = GetDlgItem("Img_B33"); if (pObj) pObj->Show(true);
			pObj = GetDlgItem("Img_EnemyR33"); if (pObj) pObj->Show(true);
		}
		else
		{
			pObj = GetDlgItem("Img_R33"); if (pObj) pObj->Show(true);
			pObj = GetDlgItem("Img_EnemyB33"); if (pObj) pObj->Show(true);
		}
	}
	else
	{
		// 6x6 mode
		if (GetHostPlayer()->GetBattleCamp() == GP_BATTLE_CAMP_DEFENDER)
		{
			pObj = GetDlgItem("Img_B66"); if (pObj) pObj->Show(true);
			pObj = GetDlgItem("Img_EnemyR66"); if (pObj) pObj->Show(true);
		}
		else
		{
			pObj = GetDlgItem("Img_R66"); if (pObj) pObj->Show(true);
			pObj = GetDlgItem("Img_EnemyB66"); if (pObj) pObj->Show(true);
		}
	}

	ACString str;

	// Set team names
	if (GetHostPlayer()->GetBattleCamp() == GP_BATTLE_CAMP_DEFENDER)
	{
		// Defender is blue team
		ACString blueName((const ACHAR*)pCmd->blue_team_name, 40 / sizeof(ACHAR));
		pObj = GetDlgItem("Txt_TeamName1"); if (pObj) pObj->SetText(blueName);
		ACString redName((const ACHAR*)pCmd->red_team_name, 40 / sizeof(ACHAR));
		pObj = GetDlgItem("Txt_TeamName2"); if (pObj) pObj->SetText(redName);
	}
	else
	{
		// Invader is red team
		ACString redName((const ACHAR*)pCmd->red_team_name, 40 / sizeof(ACHAR));
		pObj = GetDlgItem("Txt_TeamName1"); if (pObj) pObj->SetText(redName);
		ACString blueName((const ACHAR*)pCmd->blue_team_name, 40 / sizeof(ACHAR));
		pObj = GetDlgItem("Txt_TeamName2"); if (pObj) pObj->SetText(blueName);
	}

	// Parse dynamic arrays
	// Structure: fixed part + red_players[] + blue_players[]
	const unsigned char* pDataBuf = (const unsigned char*)pData;
	pDataBuf += sizeof(cmd_arena_battle_result); // Skip fixed part

	// Parse red players
	arena_player_result* pRedPlayers = (arena_player_result*)pDataBuf;
	for (int i = 0; i < pCmd->red_count_players; i++)
	{
		// Validate array bounds (defensive programming)
		if (i >= 6) break;  // Maximum 6 players per team

		ACString strPlayerName((const ACHAR*)pRedPlayers[i].name, 40 / sizeof(ACHAR));
		str.Format(_AL("%s \t %s \t %d \t %d \t %d"),
			strPlayerName,
			GetGameRun()->GetProfName(pRedPlayers[i].cls),
			pRedPlayers[i].wins,
			pRedPlayers[i].point_solo,
			pRedPlayers[i].point_team  // Synchronized with server
		);

		if (GetHostPlayer()->GetBattleCamp() == GP_BATTLE_CAMP_DEFENDER)
		{
			m_pLst2->AddString(str);
		}
		else
		{
			m_pLst1->AddString(str);
		}
	}

	// Parse blue players
	arena_player_result* pBluePlayers = (arena_player_result*)(pDataBuf + pCmd->red_count_players * sizeof(arena_player_result));
	for (int i = 0; i < pCmd->blue_count_players; i++)
	{
		// Validate array bounds (defensive programming)
		if (i >= 6) break;  // Maximum 6 players per team

		ACString strPlayerName((const ACHAR*)pBluePlayers[i].name, 40 / sizeof(ACHAR));
		str.Format(_AL("%s \t %s \t %d \t %d \t %d"),
			strPlayerName,
			GetGameRun()->GetProfName(pBluePlayers[i].cls),
			pBluePlayers[i].wins,
			pBluePlayers[i].point_solo,
			pBluePlayers[i].point_team
		);

		if (GetHostPlayer()->GetBattleCamp() == GP_BATTLE_CAMP_DEFENDER)
		{
			m_pLst1->AddString(str);
		}
		else
		{
			m_pLst2->AddString(str);
		}
	}

	// Set statistics
	if (GetHostPlayer()->GetBattleCamp() == GP_BATTLE_CAMP_DEFENDER)
	{
		// Defender (blue team) stats
		str.Format(L"%d", pCmd->blue_count_alive_players);
		pObj = GetDlgItem("Txt_Alive1"); if (pObj) pObj->SetText(str);
		str.Format(L"%d", pCmd->red_count_alive_players);
		pObj = GetDlgItem("Txt_Alive2"); if (pObj) pObj->SetText(str);

		str.Format(L"%d", pCmd->blue_damage);
		pObj = GetDlgItem("Txt_Attack1"); if (pObj) pObj->SetText(str);
		str.Format(L"%d", pCmd->red_damage);
		pObj = GetDlgItem("Txt_Attack2"); if (pObj) pObj->SetText(str);

		// Score with delta
		if (pCmd->team_win == 2) // Blue won
		{
			int delta = pCmd->blue_new_score - pCmd->blue_old_score;
			str.Format(L"%d [^ff0000+%d^fceacc]", pCmd->blue_new_score, delta);
			pObj = GetDlgItem("Txt_Score1"); if (pObj) pObj->SetText(str);
			delta = pCmd->red_new_score - pCmd->red_old_score;
			str.Format(L"%d [^ff0000%d^fceacc]", pCmd->red_new_score, delta);
			pObj = GetDlgItem("Txt_Score2"); if (pObj) pObj->SetText(str);
		}
		else if (pCmd->team_win == 1) // Red won
		{
			int delta = pCmd->blue_new_score - pCmd->blue_old_score;
			str.Format(L"%d [^ff0000%d^fceacc]", pCmd->blue_new_score, delta);
			pObj = GetDlgItem("Txt_Score1"); if (pObj) pObj->SetText(str);
			delta = pCmd->red_new_score - pCmd->red_old_score;
			str.Format(L"%d [^ff0000+%d^fceacc]", pCmd->red_new_score, delta);
			pObj = GetDlgItem("Txt_Score2"); if (pObj) pObj->SetText(str);
		}
		else // Draw
		{
			str.Format(L"%d", pCmd->blue_new_score);
			pObj = GetDlgItem("Txt_Score1"); if (pObj) pObj->SetText(str);
			str.Format(L"%d", pCmd->red_new_score);
			pObj = GetDlgItem("Txt_Score2"); if (pObj) pObj->SetText(str);
		}
	}
	else
	{
		// Invader (red team) stats
		str.Format(L"%d", pCmd->red_count_alive_players);
		pObj = GetDlgItem("Txt_Alive1"); if (pObj) pObj->SetText(str);
		str.Format(L"%d", pCmd->blue_count_alive_players);
		pObj = GetDlgItem("Txt_Alive2"); if (pObj) pObj->SetText(str);

		str.Format(L"%d", pCmd->red_damage);
		pObj = GetDlgItem("Txt_Attack1"); if (pObj) pObj->SetText(str);
		str.Format(L"%d", pCmd->blue_damage);
		pObj = GetDlgItem("Txt_Attack2"); if (pObj) pObj->SetText(str);

		// Score with delta
		if (pCmd->team_win == 1) // Red won
		{
			int delta = pCmd->red_new_score - pCmd->red_old_score;
			str.Format(L"%d [^ff0000+%d^fceacc]", pCmd->red_new_score, delta);
			pObj = GetDlgItem("Txt_Score1"); if (pObj) pObj->SetText(str);
			delta = pCmd->blue_new_score - pCmd->blue_old_score;
			str.Format(L"%d [^ff0000%d^fceacc]", pCmd->blue_new_score, delta);
			pObj = GetDlgItem("Txt_Score2"); if (pObj) pObj->SetText(str);
		}
		else if (pCmd->team_win == 2) // Blue won
		{
			int delta = pCmd->red_new_score - pCmd->red_old_score;
			str.Format(L"%d [^ff0000%d^fceacc]", pCmd->red_new_score, delta);
			pObj = GetDlgItem("Txt_Score1"); if (pObj) pObj->SetText(str);
			delta = pCmd->blue_new_score - pCmd->blue_old_score;
			str.Format(L"%d [^ff0000+%d^fceacc]", pCmd->blue_new_score, delta);
			pObj = GetDlgItem("Txt_Score2"); if (pObj) pObj->SetText(str);
		}
		else // Draw
		{
			str.Format(L"%d", pCmd->red_new_score);
			pObj = GetDlgItem("Txt_Score1"); if (pObj) pObj->SetText(str);
			str.Format(L"%d", pCmd->blue_new_score);
			pObj = GetDlgItem("Txt_Score2"); if (pObj) pObj->SetText(str);
		}
	}
}

