/*
 * FILE: EC_MemSimplify.cpp
 *
 * DESCRIPTION: 
 *
 * CREATED BY: Shizhenhua, 2013/4/3
 *
 * HISTORY: 
 *
 * Copyright (c) 2011 ACE Studio, All Rights Reserved.
 */


#include "EC_MemSimplify.h"
#include "EC_Global.h"
#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EC_World.h"
#include "EC_ManPlayer.h"
#include "EC_UIManager.h"
#include "EC_GameUIMan.h"
#include "EC_ElsePlayer.h"
#include "EC_HostPlayer.h"
#include "EC_Friend.h"
#include "EC_Team.h"
#include "EC_Faction.h"
#include "EC_UIConfigs.h"
#include "EC_Optimize.h"
#include "EC_Configs.h"

#include "DlgSettingVideo.h"
#include <psapi.h>

#define new A_DEBUG_NEW

///////////////////////////////////////////////////////////////////////////
//	
//	Define and Macro
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Reference to External variables and functions
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Local Types and Variables and Global variables
//	
///////////////////////////////////////////////////////////////////////////
enum MEMUSAGE_NOMODEL_UNIMPORTANT_MASK	//	MEMUSAGE_NOMODEL_UNIMPORTANT 状态下使用的 Mask
{
	NOMODEL_UNIMPORTANT_RELOADING_MODELS	=	0x0001,	//	已调用 CECElsePlayer::ReloadModels
};

enum MEMUSAGE_NOMODEL_MASK				//	MEMUSAGE_NOMODEL 状态下使用的 Mask
{
	NOMODEL_RELOADING_MODELS				=	0x0001,	//	已调用 CECElsePlayer::ReloadModels
};


///////////////////////////////////////////////////////////////////////////
//	
//	Local functions
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Implement CECMemSimplify
//	
///////////////////////////////////////////////////////////////////////////


// 构造函数
CECMemSimplify::CECMemSimplify()
{
	m_dwMemSize = 0;
	m_iMemUsage = MEMUSAGE_NORMAL;
	m_cntCheckMem.SetPeriod(CECUIConfig::Instance().GetGameUI().nAutoSimplifySpeed);
	m_bAutoOptimize = CECUIConfig::Instance().GetGameUI().bEnableOptimize;
	m_dwMemLow = CECUIConfig::Instance().GetGameUI().nMemoryUsageLow*1024*1024;
	m_dwMemHigh = CECUIConfig::Instance().GetGameUI().nMemoryUsageHigh*1024*1024;
}

// 获取玩家的重要等级
int CECMemSimplify::PlayerSorter::GetImportantLevel(const CECElsePlayer* pPlayer) const
{
	int iCID = pPlayer->GetCharacterID();
	CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();

	if( pHost->GetSelectedTarget() == iCID )
		return 9;
	else if( pHost->GetBuddyID() == iCID )
		return 8;
	else if( pHost->IsRelatedPlayer(iCID) )
		return 7;
	else if( pHost->GetPVPInfo().idDuelOpp == iCID )
		return 6;
	else if( pHost->GetTradePlayer() == iCID )
		return 5;
	else if( pHost->GetSpouse() == iCID )
		return 4;
	else if( pHost->GetTeam() && pHost->GetTeam()->GetMemberByID(iCID) )
		return 3;
	else if( pHost->GetFriendMan()->GetFriendByID(iCID) )
		return 2;
	else if( pHost->GetFactionID() && g_pGame->GetFactionMan()->GetMember(iCID) )
		return 1;
	else
		return 0;
}

// 用于给玩家按照重要程度排序
bool CECMemSimplify::PlayerSorter::operator () (const CECMemSimplify::PlayerQueueNode &lhs, const CECMemSimplify::PlayerQueueNode &rhs) const
{
	const CECElsePlayer* rhs1 = lhs.m_pPlayer;
	const CECElsePlayer* rhs2 = rhs.m_pPlayer;
	int iImportantLvl1 = GetImportantLevel(rhs1);
	int iImportantLvl2 = GetImportantLevel(rhs2);

	if( iImportantLvl1 != iImportantLvl2 )
	{
		return iImportantLvl1 < iImportantLvl2;
	}
	else
	{
		A3DVECTOR3 vHostPos = g_pGame->GetGameRun()->GetHostPlayer()->GetPos();
		float fDist1 = (vHostPos - rhs1->GetPos()).SquaredMagnitude();
		float fDist2 = (vHostPos - rhs2->GetPos()).SquaredMagnitude();
		if( fabs(fDist1 - fDist2) > 0.0001f )
			return fDist1 > fDist2;
		else
			return rhs1 < rhs2;
	}
}

// 响应玩家进入世界
void CECMemSimplify::OnPlayerEnter(CECElsePlayer* pPlayer)
{
	if( !pPlayer )
		return;

	pPlayer->SetMemUsage(m_iMemUsage);
	m_PlayerQueue[m_iMemUsage].push_back(pPlayer);
}

// 响应玩家离开世界
void CECMemSimplify::OnPlayerLeave(CECElsePlayer* pPlayer)
{
	for( int i=MEMUSAGE_NORMAL;i<MEMUSAGE_NUM;i++ )
	{
		PlayerQueue::iterator it = std::find(m_PlayerQueue[i].begin(), m_PlayerQueue[i].end(), pPlayer);
		if( it != m_PlayerQueue[i].end() )
		{
			m_PlayerQueue[i].erase(it);
			break;
		}
	}
}

// 更新当前状态
void CECMemSimplify::Tick(DWORD dwDeltaTime)
{
	// 只对游戏状态进行优化
	if( g_pGame->GetGameRun()->GetGameState() != CECGameRun::GS_GAME )
		return;

	if( m_bAutoOptimize && m_cntCheckMem.IncCounter(dwDeltaTime) )
	{
		// 获取当前虚拟内存
		if (CECOptimize::Instance().GetVirtualSize(m_dwMemSize)){
			if( m_dwMemSize < m_dwMemLow )
			{
				for( int i=MEMUSAGE_NUM-1;i>MEMUSAGE_NORMAL;i-- )
				{
					if( m_PlayerQueue[i].empty() )
						continue;
					
					std::sort(m_PlayerQueue[i].begin(), m_PlayerQueue[i].end(), PlayerSorter());
					TransferUsage((*m_PlayerQueue[i].rbegin()).m_pPlayer, i-1);
					break;
				}
			}
			else if( m_dwMemSize > m_dwMemHigh )
			{
				for( int i=0;i<MEMUSAGE_NUM-1;i++ )
				{
					if( m_PlayerQueue[i].empty() )
						continue;
					
					std::sort(m_PlayerQueue[i].begin(), m_PlayerQueue[i].end(), PlayerSorter());
					TransferUsage((*m_PlayerQueue[i].begin()).m_pPlayer, i+1);
					break;
				}
			}
		}

		m_cntCheckMem.Reset();
	}

	for( int i=0;i<MEMUSAGE_NUM;i++ )
	{
		if( !m_PlayerQueue[i].empty() )
		{
			m_iMemUsage = i;
			break;
		}
	}

	// 更新无模型状态的玩家
 	UpdatePlayerModels();
}

// 结束游戏状态时调用
void CECMemSimplify::OnEndGameState()
{
	m_iMemUsage = MEMUSAGE_NORMAL;
	m_bAutoOptimize = CECUIConfig::Instance().GetGameUI().bEnableOptimize;

	for( int i=MEMUSAGE_NORMAL;i<MEMUSAGE_NUM;i++ )
	{
		m_PlayerQueue[i].clear();
	}
}

// 切换所有玩家到指定的内存状态
void CECMemSimplify::TransferUsageAll(int iNewState)
{
	// 自动模式，不能手动切换状态
	if( m_bAutoOptimize )
		return;
	if (iNewState < 0 || iNewState >= MEMUSAGE_NUM){
		ASSERT(false);
		return;
	}

	CECPlayerMan* pPlayerMan = g_pGame->GetGameRun()->GetWorld()->GetPlayerMan();
	if( !pPlayerMan ) return;

	CECPlayerMan::PlayerTable::iterator it;
	CECPlayerMan::PlayerTable& playerTab = pPlayerMan->GetPlayerTable();
	for( it=playerTab.begin();it!=playerTab.end();++it )
	{
		TransferUsage(*it.value(), iNewState);
	}

	m_iMemUsage = iNewState;
}

// 切换玩家的内存状态
void CECMemSimplify::TransferUsage(CECElsePlayer* pPlayer, int iNewUsage)
{
	static FN_TRANSFER_USAGE pfnTransfer[MEMUSAGE_NUM][MEMUSAGE_NUM] =
	{
		{ NULL, &CECMemSimplify::ToggleFaceMode, &CECMemSimplify::ToggleSimpleModel, &CECMemSimplify::ToggleNoModelNoImportant, &CECMemSimplify::ToggleNoModel },					// MEMUSAGE_NORMAL -> ...
		{ &CECMemSimplify::ToggleFaceMode, NULL, &CECMemSimplify::ToggleSimpleModel, &CECMemSimplify::ToggleNoModelNoImportant, &CECMemSimplify::ToggleNoModel },					// MEMUSAGE_NOFACE -> ...
		{ &CECMemSimplify::ToggleSimpleModel, &CECMemSimplify::ToggleSimpleModel, NULL, &CECMemSimplify::ToggleNoModelNoImportant, &CECMemSimplify::ToggleNoModel },				// MEMUSAGE_SIMPLEMODEL -> ...
		{ &CECMemSimplify::ToggleNoModelNoImportant, &CECMemSimplify::ToggleNoModelNoImportant, &CECMemSimplify::ToggleNoModelNoImportant, NULL , &CECMemSimplify::ToggleNoModel},	// MEMUSAGE_NOMODEL_UNIMPORTANT -> ...
		{ &CECMemSimplify::ToggleNoModel, &CECMemSimplify::ToggleNoModel, &CECMemSimplify::ToggleNoModel, &CECMemSimplify::ToggleNoModel, NULL },									// MEMUSAGE_NOMODEL -> ...
	};
	
	if( !pPlayer || iNewUsage < MEMUSAGE_NORMAL || iNewUsage >= MEMUSAGE_NUM )
	{
		ASSERT(0);
		return;
	}

	int iOldUsage = pPlayer->GetMemUsage();
	FN_TRANSFER_USAGE pFunc = pfnTransfer[iOldUsage][iNewUsage];
	if( pFunc )
	{
		PlayerQueue::iterator it = std::find(m_PlayerQueue[iOldUsage].begin(), m_PlayerQueue[iOldUsage].end(), pPlayer);
		if( it != m_PlayerQueue[iOldUsage].end() )
			m_PlayerQueue[iOldUsage].erase(it);

		pPlayer->SetMemUsage(iNewUsage);
		(this->*pFunc)( pPlayer );
		m_PlayerQueue[iNewUsage].push_back(pPlayer);
	}
}

// 切换自定义脸的模式
void CECMemSimplify::ToggleFaceMode(CECElsePlayer* pPlayer)
{
	if( !pPlayer )
	{
		ASSERT(0);
		return;
	}

	if( !pPlayer->ShouldUseFaceModel() )
	{
		if( pPlayer->GetFaceModel() )
			pPlayer->ReleaseFaceModel();
	}
	else
	{
		if( !pPlayer->GetFaceModel() && pPlayer->GetMajorModel() )
			pPlayer->QueueLoadFace(false);
	}
}

// 切换简单模型模式
void CECMemSimplify::ToggleSimpleModel(CECElsePlayer* pPlayer)
{
	if( !pPlayer )
	{
		ASSERT(0);
		return;
	}
	
	// Skip simplification if target simple model is enabled and this is the target
	if (ShouldExcludeFromSimplification(pPlayer))
		return;

	pPlayer->SwitchSimpleModel();
	if( !pPlayer->ShouldUseClothedModel() )
	{
		// 由正常模式切换到简单模型模式，需销毁Face
		if( !pPlayer->ShouldUseFaceModel() && pPlayer->GetFaceModel() )
			pPlayer->ReleaseFaceModel();
	}
	else
	{
		// 由简单模型模式切换到正常模式，需加载Face
		if( pPlayer->ShouldUseFaceModel() && !pPlayer->GetFaceModel() && pPlayer->GetMajorModel() )
			pPlayer->QueueLoadFace(false);
	}
}

// 切换非重要玩家无模型模式
void CECMemSimplify::ToggleNoModelNoImportant(CECElsePlayer* pPlayer)
{
	if( !pPlayer )
	{
		ASSERT(0);
		return;
	}
	
	// Skip simplification if target simple model is enabled and this is the target
	if (ShouldExcludeFromSimplification(pPlayer))
		return;

	if( !pPlayer->IsBaseInfoReady() || !pPlayer->IsCustomDataReady() ||	!pPlayer->IsEquipDataReady() )
		return;
	
	// Check if player should be loaded based on limits
	if (!ShouldLoadPlayer(pPlayer))
	{
		// Limit reached, don't load model
		if (pPlayer->GetPlayerModel())
		{
			pPlayer->ReleaseModels();
		}
		return;
	}

	int iCurUsage = pPlayer->GetMemUsage();
	if( iCurUsage == MEMUSAGE_NORMAL || iCurUsage == MEMUSAGE_NOFACE )
	{
		if( pPlayer->GetPlayerModel() )
		{
			ToggleSimpleModel(pPlayer);
		}
		else
		{
			pPlayer->ReleaseModels();
			pPlayer->LoadPlayerSkeleton(false);
		}

		pPlayer->ReloadGoblin();
		pPlayer->UpdateGodEvilSprite();
	}
	else if( iCurUsage == MEMUSAGE_SIMPLEMODEL )
	{
		if( !pPlayer->GetPlayerModel() )
		{
			pPlayer->ReleaseModels();
			pPlayer->LoadPlayerSkeleton(false);
		}

		pPlayer->ReloadGoblin();
		pPlayer->UpdateGodEvilSprite();
	}
}

// 切换其他玩家无模型模式
void CECMemSimplify::ToggleNoModel(CECElsePlayer* pPlayer)
{
	ToggleNoModelNoImportant(pPlayer);
}

// 更新玩家模型状态
void CECMemSimplify::UpdatePlayerModels()
{
	LoadUnloadByImportance(MEMUSAGE_NOMODEL_UNIMPORTANT, NOMODEL_UNIMPORTANT_RELOADING_MODELS, &CECMemSimplify::IsImportant);
	LoadUnloadByImportance(MEMUSAGE_NOMODEL, NOMODEL_RELOADING_MODELS, &CECMemSimplify::IsMostImportant);
}

void CECMemSimplify::LoadUnloadByImportance(int queueIndex, DWORD dwReloadingMask, ImportanceFunction isImportant){	
	PlayerQueue::iterator it;
	for (it = m_PlayerQueue[queueIndex].begin(); it != m_PlayerQueue[queueIndex].end(); ++ it){		
		PlayerQueueNode &node = *it;
		CECElsePlayer* pPlayer = node.m_pPlayer;
		if ((this->*isImportant)(pPlayer)){
			if (pPlayer->GetPlayerModel() && pPlayer->IsClothesOn()){
				// 使用简单模型
				pPlayer->SwitchSimpleModel();
				// 删除自定义脸部
				if (pPlayer->GetFaceModel()){
					pPlayer->ReleaseFaceModel();
				}
				node.Unmask(dwReloadingMask);
			}else{
				// 重要玩家需加载模型
				if (!pPlayer->IsAllResReady()){
					if (!node.HasMask(dwReloadingMask)){
						pPlayer->ReloadModels();
						node.Mask(dwReloadingMask);
					}
				}else{
					node.Unmask(dwReloadingMask);
				}
			}
		}else{
			// Check if player should be loaded based on limits (non-important players)
			if (ShouldLoadPlayer(pPlayer)){
				// Allow loading if within limits
				if (!pPlayer->IsAllResReady()){
					if (!node.HasMask(dwReloadingMask)){
						pPlayer->ReloadModels();
						node.Mask(dwReloadingMask);
					}
				}else{
					node.Unmask(dwReloadingMask);
				}
			}else{
				// Limit reached, unload model if exists
				if (pPlayer->GetPlayerModel()){
					pPlayer->ReleaseModels();
				}			
				node.Unmask(dwReloadingMask);
			}
		}
	}
}

// 是否重要玩家
bool CECMemSimplify::IsImportant(const CECPlayer* pPlayer ) const
{
	if (!pPlayer){
		ASSERT(0);
		return false;
	}
	if (!pPlayer->IsElsePlayer()){
		return true;
	}

	int iCID = pPlayer->GetCharacterID();
	CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();
	if( pHost->GetSelectedTarget() == iCID ||
		pHost->IsRelatedPlayer(iCID) ||
		pHost->GetBuddyID() == iCID || pHost->GetCandBuddyID() == iCID ||
		pHost->GetPVPInfo().idDuelOpp == iCID ||
		pHost->GetSpouse() == iCID ||
		pHost->GetTradePlayer() == iCID ||
		pHost->GetTeam() != NULL && pHost->GetTeam()->GetMemberByID(iCID) != NULL
		)
		return true;
	else
		return false;
}

// 是否最重要玩家
bool CECMemSimplify::IsMostImportant(const CECPlayer* pPlayer ) const
{
	if (!pPlayer){
		ASSERT(0);
		return false;
	}
	if (!pPlayer->IsElsePlayer()){
		return true;
	}
	
	int iCID = pPlayer->GetCharacterID();
	CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();
	if( pHost->GetSelectedTarget() == iCID ||
		pHost->GetBuddyID() == iCID || pHost->GetCandBuddyID() == iCID ||
		pHost->GetPVPInfo().idDuelOpp == iCID)
		return true;
	else
		return false;
}

// Check if player should be loaded based on limits
bool CECMemSimplify::ShouldLoadPlayer(const CECElsePlayer* pPlayer) const
{
	if (!pPlayer)
		return false;
	
	CECConfigs* pConfigs = g_pGame ? g_pGame->GetConfigs() : NULL;
	if (!pConfigs)
		return true;
	
	const EC_OPTIMIZE_SETTING& ops = pConfigs->GetOptimizeSettings();
	
	// If people limit is not enabled, allow all players
	if (!ops.bPeopleLimit)
		return true;
	
	// Check if player is important (target, team member, etc.)
	// Important players are always loaded
	if (IsImportant(pPlayer))
		return true;
	
	// Check mate/enemy limits
	bool bIsMate = IsMate(pPlayer);
	bool bIsEnemy = IsEnemy(pPlayer);
	
	// If player is neither mate nor enemy, allow loading (neutral players)
	if (!bIsMate && !bIsEnemy)
		return true;
	
	CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();
	if (!pHost)
		return true;
	
	A3DVECTOR3 vHostPos = pHost->GetPos();
	A3DVECTOR3 vPlayerPos = pPlayer->GetPos();
	float fDist = (vHostPos - vPlayerPos).Magnitude();
	
	// If PeopleLimitByNear is enabled, prioritize nearby players
	// But we still need to check limits
	
	// Count currently loaded players of the same type (mate/enemy)
	int nLoadedCount = 0;
	int nTotalCount = 0;
	
	// Count all players in queues (both loaded and unloaded)
	for (int i = MEMUSAGE_NORMAL; i < MEMUSAGE_NUM; i++)
	{
		for (PlayerQueue::const_iterator it = m_PlayerQueue[i].begin(); it != m_PlayerQueue[i].end(); ++it)
		{
			const CECElsePlayer* pOtherPlayer = it->m_pPlayer;
			if (!pOtherPlayer || pOtherPlayer == pPlayer)
				continue;
			
			// Skip important players (they don't count towards limit)
			if (IsImportant(pOtherPlayer))
				continue;
			
			bool bOtherIsMate = IsMate(pOtherPlayer);
			bool bOtherIsEnemy = IsEnemy(pOtherPlayer);
			
			// Count only same type (mate or enemy)
			if (bIsMate && bOtherIsMate)
			{
				nTotalCount++; // Count all mates (loaded or not)
				// Count as loaded if has model
				if (pOtherPlayer->GetPlayerModel())
					nLoadedCount++;
			}
			else if (bIsEnemy && bOtherIsEnemy)
			{
				nTotalCount++; // Count all enemies (loaded or not)
				// Count as loaded if has model
				if (pOtherPlayer->GetPlayerModel())
					nLoadedCount++;
			}
		}
	}
	
	// Add current player to total count (if not already counted)
	// This ensures we account for the player we're checking
	nTotalCount++;
	
	// Check if we should load this player based on limits
	if (bIsMate && ops.nMateLimit < 100)
	{
		// Calculate max allowed mates based on percentage of total visible mates
		// If no mates visible yet, allow first one
		if (nTotalCount == 0)
			return true;
		
		// Calculate max allowed (percentage of total)
		int nMaxAllowed = (int)((nTotalCount * ops.nMateLimit) / 100.0f + 0.5f);
		if (nMaxAllowed < 1) nMaxAllowed = 1; // At least 1
		
		if (nLoadedCount >= nMaxAllowed)
		{
			// Limit reached, but check if this player is closer than others
			if (ops.bPeopleLimitByNear)
			{
				// Allow loading nearby players (within 100 meters) even if limit reached
				// This will prioritize nearby players over distant ones
				float fMaxDistance = 100.0f; // 100 meters
				if (fDist <= fMaxDistance)
				{
					// Allow loading nearby players
					return true;
				}
			}
			return false; // Limit reached
		}
	}
	
	if (bIsEnemy && ops.nEnemyLimit < 100)
	{
		// Calculate max allowed enemies based on percentage of total visible enemies
		// If no enemies visible yet, allow first one
		if (nTotalCount == 0)
			return true;
		
		// Calculate max allowed (percentage of total)
		int nMaxAllowed = (int)((nTotalCount * ops.nEnemyLimit) / 100.0f + 0.5f);
		if (nMaxAllowed < 1) nMaxAllowed = 1; // At least 1
		
		if (nLoadedCount >= nMaxAllowed)
		{
			// Limit reached, but check if this player is closer than others
			if (ops.bPeopleLimitByNear)
			{
				float fMaxDistance = 100.0f; // 100 meters
				if (fDist <= fMaxDistance)
				{
					// Allow loading nearby players
					return true;
				}
			}
			return false; // Limit reached
		}
	}
	
	return true;
}

// Check if player is mate (ally)
bool CECMemSimplify::IsMate(const CECPlayer* pPlayer) const
{
	if (!pPlayer || !pPlayer->IsElsePlayer())
		return false;
	
	CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();
	if (!pHost)
		return false;
	
	int iCID = pPlayer->GetCharacterID();
	
	// Check if same team
	if (pHost->GetTeam() && pHost->GetTeam()->GetMemberByID(iCID))
		return true;
	
	// Check if same battle camp (ally)
	if (pHost->IsInBattle() && pPlayer->IsInBattle())
	{
		int hostCamp = pHost->GetBattleCamp();
		int playerCamp = pPlayer->GetBattleCamp();
		// GP_BATTLE_CAMP_NONE is 0, so check for non-zero and equal camps
		if (hostCamp != 0 && hostCamp == playerCamp)
			return true;
	}
	
	// Check if friend
	if (pHost->GetFriendMan() && pHost->GetFriendMan()->GetFriendByID(iCID))
		return true;
	
	return false;
}

// Check if player is enemy
bool CECMemSimplify::IsEnemy(const CECPlayer* pPlayer) const
{
	if (!pPlayer || !pPlayer->IsElsePlayer())
		return false;
	
	CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();
	if (!pHost)
		return false;
	
	int iCID = pPlayer->GetCharacterID();
	
	// Check if different battle camp (enemy)
	if (pHost->IsInBattle() && pPlayer->IsInBattle())
	{
		int hostCamp = pHost->GetBattleCamp();
		int playerCamp = pPlayer->GetBattleCamp();
		// GP_BATTLE_CAMP_NONE is 0, so check for different non-zero camps
		if (hostCamp != 0 && playerCamp != 0 && hostCamp != playerCamp)
			return true;
	}
	
	// Check if PvP enemy (if PvP system has enemy list)
	// This is a placeholder - actual PvP enemy checking may be more complex
	
	return false;
}

// Check if player should be excluded from simplification (target)
bool CECMemSimplify::ShouldExcludeFromSimplification(const CECElsePlayer* pPlayer) const
{
	if (!pPlayer)
		return false;
	
	CECConfigs* pConfigs = g_pGame ? g_pGame->GetConfigs() : NULL;
	if (!pConfigs)
		return false;
	
	// If target simple model is enabled, exclude target from simplification
	if (pConfigs->GetOptimizeTargetSimpleModel())
	{
		CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();
		if (pHost && pHost->GetSelectedTarget() == pPlayer->GetCharacterID())
			return true;
	}
	
	return false;
}
