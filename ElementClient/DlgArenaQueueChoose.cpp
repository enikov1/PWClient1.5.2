// Filename: DlgArenaQueueChoose.cpp
// Arena of Aurora - Queue choose dialog implementation

#include "DlgArenaQueueChoose.h"
#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EC_GameSession.h"
#include "EC_Global.h"
#include "EC_GameUIMan.h"
#include "EC_HostPlayer.h"

AUI_BEGIN_COMMAND_MAP(CDlgArenaQueueChoose, CDlgBase)
AUI_ON_COMMAND("confirm", OnCommandConfirm)
AUI_END_COMMAND_MAP()

AUI_BEGIN_EVENT_MAP(CDlgArenaQueueChoose, CDlgBase)
AUI_END_EVENT_MAP()

CDlgArenaQueueChoose::CDlgArenaQueueChoose()
{
}

CDlgArenaQueueChoose::~CDlgArenaQueueChoose()
{
}

bool CDlgArenaQueueChoose::OnInitDialog()
{
	return true;
}

void CDlgArenaQueueChoose::OnShowDialog()
{
}

void CDlgArenaQueueChoose::OnHideDialog()
{
}

void CDlgArenaQueueChoose::OnCommandConfirm(const char* szCommand)
{
	int nIndex;
	DDX_RadioButton(true, 0, nIndex);

	// nIndex is ButtonID from XML (1, 2, or 3)
	// Convert to battle_mode: 1¡ú0 (1x1), 2¡ú1 (3x3), 3¡ú2 (6x6)
	// Server expects: battle_mode 0 = 1x1 (solo), 1 = 3x3 (team), 2 = 6x6 (team)
	
	if (nIndex < 0)
		return;  // No radio button selected
	
	CECGameUIMan* pGameUI = GetGameUIMan();
	if (!pGameUI)
	{
		Show(false);
		return;
	}
	
	// Check if queue or battle is already active
	CECHostPlayer* pHost = GetHostPlayer();
	bool bArenaActive = false;
	
	if (pHost && pHost->GetBattleInfo().nType == CECHostPlayer::BT_ARENA)
	{
		bArenaActive = true;
	}
	
	if (!bArenaActive)
	{
		PAUIDIALOG pDlgQueue = pGameUI->GetDialog("Win_ArenaQueue");
		if (pDlgQueue && pDlgQueue->IsShow())
		{
			bArenaActive = true;
		}
	}
	
	if (!bArenaActive)
	{
		PAUIDIALOG pDlgMessage = pGameUI->GetDialog("Win_ArenaMessage");
		if (pDlgMessage && pDlgMessage->IsShow())
		{
			bArenaActive = true;
		}
	}
	
	if (!bArenaActive)
	{
		PAUIDIALOG pDlgPoint = pGameUI->GetDialog("Win_ArenaPoint");
		if (pDlgPoint && pDlgPoint->IsShow())
		{
			bArenaActive = true;
		}
	}
	
	if (bArenaActive)
	{
		ACString strMsg = pGameUI->GetStringFromTable(13579);
		pGameUI->AddChatMessage(strMsg, GP_CHAT_SYSTEM);
		Show(false);
		return;
	}
	
	int battle_mode = nIndex - 1;
	
	if (battle_mode >= 0 && battle_mode <= 2)
	{
		g_pGame->GetGameSession()->c2s_SendCmdArenaRequestBattleQueue(battle_mode);
	}
	Show(false);
}

