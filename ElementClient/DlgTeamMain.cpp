// File		: DlgTeamMain.cpp
// Creator	: Xiao Zhou
// Date		: 2005/8/17

#include "AFI.h"
#include "A2DSprite.h"
#include "DlgTeamMain.h"
#include "DlgTeamMate.h"
#include "DlgHost.h"
#include "EC_GameRun.h"
#include "EC_GameUIMan.h"
#include "EC_HostPlayer.h"
#include "EC_Team.h"
#include "EC_World.h"
#include "EC_ManPlayer.h"
#include "EC_Global.h"
#include "EC_Configs.h"
#include "EC_Utility.h"
#include "EC_GameSession.h"
#include "EC_InstanceReenter.h"
#include "GT/gt_overlay.h"
#include "ElementSkill.h"
#include "AUILabel.h"
#include "AUICommon.h"
#include "AUIProgress.h"
#include "AUIImagePicture.h"

#define new A_DEBUG_NEW
#define CDLGTEAMMAIN_MAXTEAMMATES		10

AUI_BEGIN_COMMAND_MAP(CDlgTeamMain, CDlgBase)

AUI_ON_COMMAND("IDCANCEL",	OnCommandCANCAL)
AUI_ON_COMMAND("minimize",	OnCommandMinimize)
AUI_ON_COMMAND("minimizebuff",	OnCommandMinimizeBuff)
AUI_ON_COMMAND("arrow",		OnCommandArrow)
AUI_ON_COMMAND("Btn_GTalk",	OnCommandGTTalk)
AUI_ON_COMMAND("kwi_reformsquad",	OnCommandReformSquad)

AUI_END_COMMAND_MAP()


CDlgTeamMain::CDlgTeamMain()
{
	m_bShowTeamArrow = true;
	m_bShowBuff = true;
	m_bGTWaiting = false;
	m_bReformInProgress = false;
	m_dwLastInviteTime = 0;
	m_nCurrentInviteIndex = 0;
}

CDlgTeamMain::~CDlgTeamMain()
{
}

void CDlgTeamMain::OnCommandCANCAL(const char *szCommand)
{
}

void CDlgTeamMain::OnCommandMinimize(const char *szCommand)
{
	int nStatus = (int)GetData();
	if( nStatus == TEAM_STATUS_OPEN )
		SetData(TEAM_STATUS_CLOSE);
	else
		SetData(TEAM_STATUS_OPEN);
}

void CDlgTeamMain::OnCommandMinimizeBuff(const char *szCommand)
{
	m_bShowBuff = !m_bShowBuff;
}

void CDlgTeamMain::OnCommandArrow(const char *szCommand)
{
	m_bShowTeamArrow = !m_bShowTeamArrow;
}

void CDlgTeamMain::OnCommandGTTalk(const char *szCommand)
{
	if (g_pGame->GetConfigs()->IsMiniClient())
	{
		GetGameUIMan()->MessageBox("", GetStringFromTable(10713), MB_OK, A3DCOLORRGBA(255, 255, 255, 160));
		return;
	}

	//	GT已登录时，切换进出 Team 频道
	//	GT未登录时，单次触发 GTWaiting 状态
	if (m_bGTWaiting) return;

	CECTeam *pTeam = GetHostPlayer()->GetTeam();
	if (!pTeam) return;

	using namespace overlay;
	if (!GTOverlay::Instance().IsLogin())
	{
		if (!GetGameUIMan()->SSOGetGTTicket())
			return;
		m_bGTWaiting = true;
		a_LogOutput(1, "CDlgTeamMain::OnCommandGTTalk, GTWaiting open");
		return;
	}
	
	if (!GTOverlay::Instance().IsInTeam())
		GTOverlay::Instance().EnterTeam(0);
	else
		GTOverlay::Instance().LeaveTeam();
}

void CDlgTeamMain::ProcessGTEnter()
{
	//	非 GTWaiting 时，直接忽略
	//	GTWaiting 时，清除 GTWating 状态并调用进入 Team 频道
	if (!m_bGTWaiting)	return;
	
	m_bGTWaiting = false;
	a_LogOutput(1, "CDlgTeamMain::ProcessGTEnter, GTWaiting cleared");
	
	using namespace overlay;
	if (!GTOverlay::Instance().IsInTeam())
		GTOverlay::Instance().EnterTeam(0);
}

bool CDlgTeamMain::UpdateTeamInfo()
{
	CECWorld *pWorld = GetWorld();
	CECHostPlayer *pHost = GetHostPlayer();
	CECTeam *pTeam = pHost->GetTeam();
	int nNumMembers = pTeam ? pTeam->GetMemberNum() : 0;

	PAUIDIALOG pDlgLast = GetGameUIMan()->m_pDlgHost;
	if( nNumMembers > 0 )
	{
		if( !IsShow() )
		{
			Show(true, false, false);
		}
		pDlgLast = this;
	}
	else if( IsShow() )
		Show(false);

	ACHAR szText[256];
	AString strFile;
	CDlgTeamMate *pDlgMate;
	PAUIPROGRESS pProgress;
	CECTeamMember *pMember;
	PAUIIMAGEPICTURE pImage;
	int i, idMateDlg = 1, idPlayer;
	int nStatus = (int)GetData();
	CECPlayerMan *pPlayerMan = pWorld->GetPlayerMan();
	
	for( i = 0; i < nNumMembers; i++ )
	{
		pMember = pTeam->GetMemberByIndex(i);
		idPlayer = pMember->GetCharacterID();
		if( pHost->GetCharacterID() == idPlayer )
			continue;

		pDlgMate = GetTeamMateDlg(idMateDlg - 1);
		if( nStatus == TEAM_STATUS_CLOSE )
		{
			if( pDlgMate->IsShow() )
				pDlgMate->Show(false);
		}
		else
		{
			PAUIIMAGEPICTURE pWallow = (PAUIIMAGEPICTURE)pDlgMate->GetDlgItem("Img_Captivation");
			if( pMember->GetWallowLevel() >= 0 )
			{
				pWallow->Show(true);
				pWallow->FixFrame(pMember->GetWallowLevel());
			}
			else
				pWallow->Show(false);

			pDlgMate->RefreshForceStatus(pMember);

			pImage = pDlgMate->m_pImgProf;
			int count = pMember->GetRenicarnationCount();
			ACString strReincarnation;
			if (count) {
				strReincarnation = _AL("\r");
				strReincarnation += GetStringFromTable(10800 + count);
			}
			ACString strPro = GetGameRun()->GetProfName(pMember->GetProfession());
			pImage->SetHint(strPro + strReincarnation);
			pImage->FixFrame(pMember->GetProfession() + pMember->GetGender() * NUM_PROFESSION);

			CECElsePlayer* pTeamElseMember = GetWorld()->GetPlayerMan()->GetElsePlayer(pMember->GetCharacterID());
			
			if (!pTeamElseMember) {
				pImage->SetColor(A3DCOLORRGBA(128, 128, 128, 64));
			}
			else {
				pImage->SetColor(A3DCOLORRGBA(255, 255, 255, 128));
			}
			PAUIIMAGEPICTURE pProfit = (PAUIIMAGEPICTURE)pDlgMate->GetDlgItem("Img_Fatigue");
			if( pMember->GetProfitLevel() == PROFIT_LEVEL_NORMAL )
			{
				pProfit->FixFrame(0);
				pProfit->SetHint(GetGameUIMan()->GetStringFromTable(9660));
				pProfit->SetFlash(false);
			}
			else if(pMember->GetProfitLevel() == PROFIT_LEVEL_NONE)
			{
				pProfit->FixFrame(2);
				pProfit->SetHint(GetGameUIMan()->GetStringFromTable(9661));
				pProfit->SetFlash(true);
			}
			else if (pMember->GetProfitLevel() == PROFIT_LEVEL_YELLOW)
			{
				pProfit->FixFrame(1);
				pProfit->SetHint(GetGameUIMan()->GetStringFromTable(9672));
			}

			AUI_ConvertChatString(pMember->GetName(), szText);
			pDlgMate->m_pTxtCharName->SetText(szText);

			a_sprintf(szText, _AL("%3d"), pMember->GetLevel());
			pDlgMate->m_pTxtLV->SetText(szText);

			a_sprintf(szText, _AL("HP: %d/%d\rMP: %d/%d"),
				pMember->GetCurHP(), pMember->GetMaxHP(),
				pMember->GetCurMP(), pMember->GetMaxMP());

			pProgress = pDlgMate->m_pPrgsHP;
			pProgress->SetProgress(pMember->GetCurHP()
				* AUIPROGRESS_MAX / max(pMember->GetMaxHP(), 1));
			pProgress->SetHint(szText);
			m_pTxt_hpteam = (PAUILABEL)pDlgMate->GetDlgItem("TXT_HPTEAM");
			
			// Check if team HP display is enabled
			bool bShowTeamHP = g_pGame && g_pGame->GetConfigs() ? 
				g_pGame->GetConfigs()->GetUISettings().bShowTeamHP : true;
			
			if (m_pTxt_hpteam)
			{
				if (bShowTeamHP)
				{
					ACString str;
					str.Format(L"%d/%d", pMember->GetCurHP(), pMember->GetMaxHP());
					m_pTxt_hpteam->SetText(str);
					m_pTxt_hpteam->Show(true);
				}
				else
				{
					m_pTxt_hpteam->Show(false);
				}
			}

			pProgress = pDlgMate->m_pPrgsMP;
			pProgress->SetProgress(pMember->GetCurMP()
				* AUIPROGRESS_MAX / max(pMember->GetMaxMP(), 1));
			pProgress->SetHint(szText);

			if( !pDlgMate->IsShow() )
				pDlgMate->Show(true, false, false);

			pDlgMate->AlignTo(pDlgLast,
				AUIDIALOG_ALIGN_INSIDE, AUIDIALOG_ALIGN_LEFT,
				AUIDIALOG_ALIGN_OUTSIDE, AUIDIALOG_ALIGN_BOTTOM);
			pDlgMate->SetCanMove(false);
			pDlgMate->SetData(pMember->GetCharacterID());

			pImage = pDlgMate->m_pImgCombatMask;
			if( pMember->IsFighting() )
			{
				pImage->Show(true);
				pImage->FadeInOut(1500);
			}
			else
			{
				pImage->Show(false);
				pImage->FadeInOut(0);
			}
			
			if( 1 == idMateDlg )	// Team leader must be the first one.
			{
				if( pTeam && pTeam->GetLeaderID() == idPlayer )
					pDlgMate->m_pImgLeader->Show(true);
				else
					pDlgMate->m_pImgLeader->Show(false);
			}

			CECIconStateMgr* pMgr = GetGameUIMan()->GetIconStateMgr();
			pMgr->RefreshStateIcon(	pDlgMate->m_pImgSt, CDLGTEAMMATE_ST_MAX, 
									m_bShowBuff ? &pMember->GetIconStates() : NULL,
									IconLayoutVertical(2), true );

			pDlgLast = pDlgMate;
		}

		idMateDlg++;
	}

	for( i = idMateDlg; i < CDLGTEAMMAIN_MAXTEAMMATES; i++ )
	{
		pDlgMate = GetTeamMateDlg(idMateDlg - 1);
		if( pDlgMate->IsShow() )
			pDlgMate->Show(false);

		idMateDlg++;
	}

	// Process reform squad invitations if in progress (after team is disbanded)
	if (m_bReformInProgress && nNumMembers == 0)
	{
		ProcessReformSquad();
	}

	return true;
}

void CDlgTeamMain::OnCommandReformSquad(const char *szCommand)
{
	CECHostPlayer *pHost = GetHostPlayer();
	CECTeam *pTeam = pHost->GetTeam();
	
	if (!pTeam)
	{
		GetGameUIMan()->AddChatMessage(GetStringFromTable(204), GP_CHAT_SYSTEM);
		return;
	}
	
	// Check if already in progress
	if (m_bReformInProgress)
	{
		return;
	}
	
	int idHost = pHost->GetCharacterID();
	int idLeader = pTeam->GetLeaderID();
	bool bIsLeader = (idLeader == idHost);
	
	// Save list of team members (excluding self)
	m_aReformMemberList.clear();
	int nNumMembers = pTeam->GetMemberNum();
	for (int i = 0; i < nNumMembers; i++)
	{
		CECTeamMember *pMember = pTeam->GetMemberByIndex(i);
		int idMember = pMember->GetCharacterID();
		if (idMember != idHost)
		{
			m_aReformMemberList.push_back(idMember);
		}
	}
	
	if (m_aReformMemberList.empty())
	{
		GetGameUIMan()->AddChatMessage(GetStringFromTable(204), GP_CHAT_SYSTEM);
		return;
	}
	
	// Initialize reform process
	m_bReformInProgress = true;
	m_nCurrentInviteIndex = 0;
	m_dwLastInviteTime = 0;
	
	// Disband team if leader, or leave if not leader
	if (bIsLeader)
	{
		// Leader disbands the team
		GetGameSession()->c2s_CmdTeamDismissParty();
	}
	else
	{
		// Not leader, just leave
		GetGameSession()->c2s_CmdTeamLeaveParty();
	}
	
	// Try to reload instance if available (check if reenter is possible)
	// Note: This will only work if player left instance and server sent INSTANCE_REENTER_NOTIFY
	// The reenter notification is handled automatically by EC_InstanceReenter, so we don't need to do anything here
	// The instance will be reentered automatically if the notification is available
}

void CDlgTeamMain::ProcessReformSquad()
{
	if (!m_bReformInProgress)
		return;
	
	CECHostPlayer *pHost = GetHostPlayer();
	CECTeam *pTeam = pHost->GetTeam();
	
	// If we're still in a team, wait until it's disbanded
	if (pTeam)
		return;
	
	// Check if we have members to invite
	if (m_aReformMemberList.empty() || m_nCurrentInviteIndex >= (int)m_aReformMemberList.size())
	{
		// All invitations sent, reset state
		m_bReformInProgress = false;
		m_aReformMemberList.clear();
		m_nCurrentInviteIndex = 0;
		m_dwLastInviteTime = 0;
		return;
	}
	
	// Send invitations with delay between them (200ms = ~12 frames at 60fps)
	DWORD dwCurrentTime = timeGetTime();
	if (m_dwLastInviteTime == 0 || (dwCurrentTime - m_dwLastInviteTime) >= 200)
	{
		// Send invitation to next member
		int idMember = m_aReformMemberList[m_nCurrentInviteIndex];
		GetGameSession()->c2s_CmdTeamInvite(idMember);
		
		m_dwLastInviteTime = dwCurrentTime;
		m_nCurrentInviteIndex++;
		
		// If all invitations sent, reset after a short delay
		if (m_nCurrentInviteIndex >= (int)m_aReformMemberList.size())
		{
			// Will be reset on next call when all invitations are sent
		}
	}
}

CDlgTeamMate* CDlgTeamMain::GetTeamMateDlg(int index)
{
	// starts from 0
	if(index >= 0 && index < (int)m_pMates.size())
	{
		return m_pMates[index];
	}

	return NULL;
}

bool CDlgTeamMain::OnInitDialog()
{
	if (!CDlgBase::OnInitDialog())
		return false;

	m_pMates.clear();
	while(true)
	{
		// starts from 1 
		AString strDlgName;
		strDlgName.Format("Win_TeamMate%d", m_pMates.size() + 1);
		CDlgTeamMate * pDlg = dynamic_cast<CDlgTeamMate*>(m_pAUIManager->GetDialog(strDlgName));
		if(!pDlg) break;
		
		m_pMates.push_back(pDlg);
	}

	return true;
}