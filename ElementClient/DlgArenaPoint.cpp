// Filename: DlgArenaPoint.cpp
// Arena of Aurora - Battle point dialog implementation

#include "AFI.h"
#include "A2DSprite.h"
#include "DlgArenaPoint.h"
#include "ArenaUtils.h"
#include "DlgEnemyTeamMate.h"
#include "DlgArenaCountTime.h"

#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EC_GameSession.h"
#include "EC_Global.h"
#include "EC_GameUIMan.h"
#include "EC_HostPlayer.h"
#include "EC_UIManager.h"
#include "EC_World.h"
#include "EC_ManPlayer.h"
#include "EC_Configs.h"
#include "EC_Utility.h"
#include "EC_ElsePlayer.h"
#include "DlgHost.h"
#include "AUILabel.h"
#include "AUICommon.h"
#include "AUIProgress.h"
#include "AUIImagePicture.h"

#define CDLGENEMY_MAXTEAMMATES 6

AUI_BEGIN_COMMAND_MAP(CDlgArenaPoint, CDlgBase)
AUI_ON_COMMAND("minimize", OnCommandMinimize)
AUI_ON_COMMAND("IDCANCEL", OnCommandCANCAL)
AUI_END_COMMAND_MAP()

CDlgArenaPoint::CDlgArenaPoint()
{
	m_iEndBattleTimestamp = 0;
	m_nInitialTime = 0;
	m_nTimeRest = -80;   // Initial value: preparation phase (synced with server ARENA_PREPARE_TIME)
	m_nRedTeamCount = 0;
	m_nBlueTeamCount = 0;
	m_iSavedBattleCamp = 0;  // Will be set when entering arena
	m_pTxt_red = NULL;
	m_pTxt_blue = NULL;
	m_pTxt_time = NULL;
}

CDlgArenaPoint::~CDlgArenaPoint()
{
}

void CDlgArenaPoint::OnCommandCANCAL(const char* szCommand)
{
}

bool CDlgArenaPoint::OnInitDialog()
{
	if (!CDlgBase::OnInitDialog())
		return false;

	m_pTxt_red = (PAUILABEL)GetDlgItem("Txt_Red");
	m_pTxt_blue = (PAUILABEL)GetDlgItem("Txt_Blue");
	m_pTxt_time = (PAUILABEL)GetDlgItem("Txt_Time");

	// Do NOT pre-load dialogs here - they will be loaded dynamically in UpdateEnemyTeamInfo()
	m_pEnemyMates.clear();
	m_pEnemyList.clear();

	return true;
}

void CDlgArenaPoint::OnTick()
{
	if (m_pTxt_time)
	{
		// Use server-synchronized time_rest
		// time_rest < 0: Preparation phase (before battle starts)
		// time_rest == 0: Battle starts
		// time_rest > 0: Battle in progress
		int nTimeRemaining = m_nTimeRest;
		bool isBattleActive = (nTimeRemaining >= 0);
		
		// For display, show absolute value
		int nDisplayTime = abs(nTimeRemaining);
		int nMin = nDisplayTime / 60;
		int nSec = nDisplayTime % 60;
		
		ACString strTxt;
		strTxt.Format(TEXT("%02d:%02d"), nMin, nSec);
		m_pTxt_time->SetText(strTxt);
		
		// Green color when battle is active (time_rest >= 0)
		// Yellow/beige color during preparation (time_rest < 0)
		if (isBattleActive)
			m_pTxt_time->SetColor(A3DCOLORRGB(0, 255, 0));  // Green
		else
			m_pTxt_time->SetColor(A3DCOLORRGB(252, 234, 204));  // Beige

		// Show countdown dialog (3, 2, 1) when battle is about to start
		CECGameUIMan* pGameUI = GetGameUIMan();
		if (pGameUI)
		{
			CDlgArenaCountTime* pDlg = dynamic_cast<CDlgArenaCountTime*>(pGameUI->GetDialog("Win_ArenaCountTime"));
			if (pDlg)
			{
				if (!isBattleActive && nDisplayTime <= 3 && nDisplayTime > 0)
				{
					pDlg->Show(true);
				}
				else if (isBattleActive || nDisplayTime == 0)
				{
					pDlg->Show(false);
				}
			}
		}
	}

	UpdateEnemyTeamInfo();
}

bool CDlgArenaPoint::UpdateEnemyTeamInfo()
{
	CECGameUIMan* pGameUI = GetGameUIMan();
	if (!pGameUI)
		return false;

	CECWorld* pWorld = GetWorld();
	if (!pWorld)
		return false;

	CECHostPlayer* pHost = GetHostPlayer();
	if (!pHost)
		return false;

	CECPlayerMan* pPlayerMan = pWorld->GetPlayerMan();
	if (!pPlayerMan)
		return false;

	int nStatus = (int)GetData();
	ACHAR szText[256];
	
	// Build new list of enemy mate dialogs (dynamic loading each tick like in backup)
	abase::vector<CDlgEnemyTeamMate*> newEnemyMates;
	PAUIDIALOG pDlgLast = this;  // Start alignment from this dialog (Win_ArenaPoint)
	
	int nIndex = 0;
	for (abase::hash_map<int, CECElsePlayer*>::iterator it = m_pEnemyList.begin();
		 it != m_pEnemyList.end() && nIndex < CDLGENEMY_MAXTEAMMATES;
		 ++it, ++nIndex)
	{
		int roleid = it->first;
		CECElsePlayer* pMember = pPlayerMan->GetElsePlayer(roleid);
		
		// Dynamically get dialog by name (dialog names start from 1)
		char szDialogName[64];
		sprintf(szDialogName, "Win_EnemyTeamMate%d", nIndex + 1);
		CDlgEnemyTeamMate* pDlgMate = dynamic_cast<CDlgEnemyTeamMate*>(pGameUI->GetDialog(szDialogName));
		
		if (!pDlgMate)
			continue;

		if (nStatus == TEAM_STATUS_CLOSE)
		{
			if (pDlgMate->IsShow())
				pDlgMate->Show(false);
		}
		else
		{
			// Get player name - works even when player is not visible
			ACString strName = GetGameRun()->GetPlayerName(roleid, true);
			if (pDlgMate->m_pTxtCharName)
				pDlgMate->m_pTxtCharName->SetText(strName);

			// Get cached server data for this enemy
			ArenaEnemyProp* pCachedProp = NULL;
			abase::hash_map<int, ArenaEnemyProp>::iterator propIt = m_EnemyProps.find(roleid);
			if (propIt != m_EnemyProps.end())
				pCachedProp = &propIt->second;

			// Use visible player data if available, otherwise use cached server data
			int nLevel = 0;
			int nHP = 0, nMaxHP = 1, nMP = 0, nMaxMP = 1;
			int nProfession = 0;
			bool bCombatState = false;

			if (pMember)
			{
				// Player is visible - use live data
				nLevel = pMember->GetBasicProps().iLevel;
				nHP = pMember->GetBasicProps().iCurHP;
				nMaxHP = max(pMember->GetExtendProps().bs.max_hp, 1);
				nMP = pMember->GetBasicProps().iCurMP;
				nMaxMP = max(pMember->GetExtendProps().bs.max_mp, 1);
				nProfession = pMember->GetProfession();
				bCombatState = pMember->IsFighting();

				// Update profession icon with gender
				PAUIIMAGEPICTURE pImage = pDlgMate->m_pImgProf;
				if (pImage)
				{
					ACString strPro = GetGameRun()->GetProfName(nProfession);
					pImage->SetHint(strPro);
					pImage->FixFrame(nProfession + pMember->GetGender() * NUM_PROFESSION);
				}

				// Update buff/debuff icons (only when visible)
				CECIconStateMgr* pMgr = pGameUI->GetIconStateMgr();
				if (pMgr)
				{
					pMgr->RefreshStateIcon(pDlgMate->m_pImgSt, CDLGTEAMMATE_ST_MAX,
						&pMember->GetIconStates(),
						IconLayoutVertical(2), true);
				}
			}
			else if (pCachedProp)
			{
				// Player not visible - use cached server data
				nLevel = pCachedProp->level;
				nHP = pCachedProp->hp;
				nMaxHP = max(pCachedProp->max_hp, 1);
				nMP = pCachedProp->mp;
				nMaxMP = max(pCachedProp->max_mp, 1);
				nProfession = pCachedProp->cls;
				bCombatState = pCachedProp->combat_state;

				// Update profession icon (no gender info in cached data, use profession only)
				PAUIIMAGEPICTURE pImage = pDlgMate->m_pImgProf;
				if (pImage)
				{
					ACString strPro = GetGameRun()->GetProfName(nProfession);
					pImage->SetHint(strPro);
					pImage->FixFrame(nProfession);
				}
			}

			// Update level
			if (pDlgMate->m_pTxtLV)
			{
				a_sprintf(szText, _AL("%3d"), nLevel);
				pDlgMate->m_pTxtLV->SetText(szText);
			}

			// Update HP/MP hint and progress bars
			a_sprintf(szText, _AL("HP: %d/%d\rMP: %d/%d"), nHP, nMaxHP, nMP, nMaxMP);

			if (pDlgMate->m_pPrgsHP)
			{
				pDlgMate->m_pPrgsHP->SetProgress(nHP * AUIPROGRESS_MAX / nMaxHP);
				pDlgMate->m_pPrgsHP->SetHint(szText);
			}

			if (pDlgMate->m_pPrgsMP)
			{
				pDlgMate->m_pPrgsMP->SetProgress(nMP * AUIPROGRESS_MAX / nMaxMP);
				pDlgMate->m_pPrgsMP->SetHint(szText);
			}

			// Combat mask effect
			if (pDlgMate->m_pImgCombatMask)
			{
				if (bCombatState)
				{
					pDlgMate->m_pImgCombatMask->Show(true);
					pDlgMate->m_pImgCombatMask->FadeInOut(1500);
				}
				else
				{
					pDlgMate->m_pImgCombatMask->Show(false);
					pDlgMate->m_pImgCombatMask->FadeInOut(0);
				}
			}

			// Show dialog and align to previous one
			if (!pDlgMate->IsShow())
				pDlgMate->Show(true, false, false);

			// Align this dialog below the previous one (chain them together)
			pDlgMate->AlignTo(pDlgLast,
				AUIDIALOG_ALIGN_INSIDE, AUIDIALOG_ALIGN_RIGHT,
				AUIDIALOG_ALIGN_OUTSIDE, AUIDIALOG_ALIGN_BOTTOM);
			pDlgMate->SetCanMove(false);
			pDlgMate->SetData(roleid);

			pDlgLast = pDlgMate;
			newEnemyMates.push_back(pDlgMate);
		}
	}

	// Hide dialogs that are no longer in the enemy list
	for (size_t i = 0; i < m_pEnemyMates.size(); i++)
	{
		bool bFound = false;
		for (size_t j = 0; j < newEnemyMates.size(); j++)
		{
			if (m_pEnemyMates[i] == newEnemyMates[j])
			{
				bFound = true;
				break;
			}
		}
		if (!bFound && m_pEnemyMates[i])
		{
			m_pEnemyMates[i]->Show(false);
		}
	}

	m_pEnemyMates = newEnemyMates;

	return true;
}

void CDlgArenaPoint::SetData(int state)
{
	// Initialize arena battle data for new battle
	// state parameter is not currently used, but kept for compatibility
	
	// IMPORTANT: Set dialog status to OPEN to ensure enemy panels are visible
	CDlgBase::SetData(TEAM_STATUS_OPEN);
	
	// Reset time to preparation phase (will be updated by server ARENA_SCORE_UPDATE)
	m_nTimeRest = -80;
	
	// Reset team counts
	m_nRedTeamCount = 0;
	m_nBlueTeamCount = 0;
	
	// Clear enemy lists for new battle
	m_pEnemyList.clear();
	m_EnemyProps.clear();
	
	// Note: Do NOT reset m_iSavedBattleCamp here - it will be set in ENTER_ARENA_BATTLEGROUND
	
	// Reset player counts display
	if (m_pTxt_red)
		m_pTxt_red->SetText(TEXT("0"));
	if (m_pTxt_blue)
		m_pTxt_blue->SetText(TEXT("0"));
}

void CDlgArenaPoint::UpdateTeamCounts(int redCount, int blueCount)
{
	m_nRedTeamCount = redCount;
	m_nBlueTeamCount = blueCount;
	
	ACString strTxt;
	if (m_pTxt_red)
	{
		strTxt.Format(TEXT("%d"), redCount);
		m_pTxt_red->SetText(strTxt);
	}
	if (m_pTxt_blue)
	{
		strTxt.Format(TEXT("%d"), blueCount);
		m_pTxt_blue->SetText(strTxt);
	}
}

void CDlgArenaPoint::OnCommandMinimize(const char* szCommand)
{
	// TODO: Implement minimize functionality
}

void CDlgArenaPoint::HideEnemiesDlg()
{
	// Hide all enemy dialogs and clear the list (dynamic loading will recreate them)
	for (size_t i = 0; i < m_pEnemyMates.size(); i++)
	{
		if (m_pEnemyMates[i])
		{
			m_pEnemyMates[i]->Show(false);
		}
	}
	m_pEnemyMates.clear();
}

CDlgEnemyTeamMate* CDlgArenaPoint::GetEnemyTeamMateDlg(int index)
{
	if (index >= 0 && index < (int)m_pEnemyMates.size())
		return m_pEnemyMates[index];
	return NULL;
}

void CDlgArenaPoint::UpdateEnemyProp(int roleid, short level, bool combat_state, short cls, int hp, int max_hp, int mp, int max_mp)
{
	ArenaEnemyProp prop;
	prop.roleid = roleid;
	prop.level = level;
	prop.combat_state = combat_state;
	prop.cls = cls;
	prop.hp = hp;
	prop.max_hp = max_hp;
	prop.mp = mp;
	prop.max_mp = max_mp;
	
	m_EnemyProps[roleid] = prop;
}
