/*
 * FILE: EC_HPWorkSpell.cpp
 *
 * DESCRIPTION: 
 *
 * CREATED BY: Duyuxin, 2004/3/8
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include "EC_Global.h"
#include "EC_HPWorkSpell.h"
#include "EC_HostPlayer.h"
#include "EC_Skill.h"
#include "EC_AutoPolicy.h"
#include "EC_PlayerWrapper.h"
#include "EC_ManAttacks.h"
#include "EC_GameRun.h"
#include "EC_World.h"
#include "EC_Configs.h"
#include "EC_Player.h"
#include "EC_ElsePlayer.h"
#include "EC_NPC.h"

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


///////////////////////////////////////////////////////////////////////////
//	
//	Local functions
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Implement CECHPWorkSpell
//	
///////////////////////////////////////////////////////////////////////////

CECHPWorkSpell::CECHPWorkSpell(CECHPWorkMan* pWorkMan) :
CECHPWork(WORK_SPELLOBJECT, pWorkMan)
{
	m_dwMask		= MASK_SPELLOBJECT;
	m_dwTransMask	= MASK_STAND | MASK_MOVETOPOS | MASK_TRACEOBJECT;

	CECHPWorkSpell::Reset();
}

CECHPWorkSpell::~CECHPWorkSpell()
{
}

//	Reset work
void CECHPWorkSpell::Reset()
{
	CECHPWork::Reset();

	m_OverTimeCnt.Reset();

	m_pSkill	= NULL;
	m_iState	= ST_INCANT;
	m_idTarget	= 0;
}

//	Copy work data
bool CECHPWorkSpell::CopyData(CECHPWork* pWork)
{
	if (!CECHPWork::CopyData(pWork))
		return false;

	CECHPWorkSpell* pSrc = (CECHPWorkSpell*)pWork;

	m_pSkill	= pSrc->m_pSkill;
	m_iState	= pSrc->m_iState;
	m_idTarget	= pSrc->m_idTarget;

	m_OverTimeCnt.SetPeriod(pSrc->m_OverTimeCnt.GetPeriod());
	m_OverTimeCnt.SetCounter(pSrc->m_OverTimeCnt.GetCounter());
	
	return true;
}

//	Prepare cast
void CECHPWorkSpell::PrepareCast(int idTarget, CECSkill* pSkill, int iIncantTime)
{
	ASSERT(pSkill);
	m_pSkill	= pSkill;
	m_iState	= ST_INCANT;
	m_idTarget	= idTarget;

	if (iIncantTime < 0)
		iIncantTime = 0x7fffffff;
	else
		iIncantTime *= 2;

	m_OverTimeCnt.SetPeriod(iIncantTime);
	m_OverTimeCnt.Reset();
}

//	Change state
void CECHPWorkSpell::ChangeState(int iState)
{
	m_iState = iState;
}

//	On first tick
void CECHPWorkSpell::OnFirstTick()
{
	m_pHost->m_iMoveMode = CECHostPlayer::MOVE_STAND;
}

void CECHPWorkSpell::Cancel()
{
	m_pHost->TurnFaceTo(0);
	CECHPWork::Cancel();
}

//	Tick routine
bool CECHPWorkSpell::Tick(DWORD dwDeltaTime)
{
	CECHPWork::Tick(dwDeltaTime);

	if (m_OverTimeCnt.IncCounter(dwDeltaTime))
		m_bFinished = true;

	// Check if auto-complete is enabled (only during spell casting, not incantation)
	bool bAutoComplete = g_pGame && g_pGame->GetConfigs() ? 
		g_pGame->GetConfigs()->GetGameSettings().bAutoComplete : false;
	
	if (bAutoComplete && m_idTarget && m_pSkill && m_iState == ST_SPELL)
	{
		CECObject* pTarget = NULL;
		float fDist = 0.0f;
		
		// Get target object and distance
		if (m_pHost->CalcDist(m_idTarget, &fDist, &pTarget))
		{
			// Check if target is dead
			if (pTarget)
			{
				bool bIsDead = false;
				if (ISNPCID(m_idTarget))
				{
					CECNPC* pNPC = dynamic_cast<CECNPC*>(pTarget);
					if (pNPC)
						bIsDead = pNPC->IsDead();
				}
				else if (ISPLAYERID(m_idTarget))
				{
					CECElsePlayer* pPlayer = dynamic_cast<CECElsePlayer*>(pTarget);
					if (pPlayer)
						bIsDead = pPlayer->IsDead();
				}
				
				if (bIsDead)
				{
					m_bFinished = true;
					return true;
				}
			}
			
			// Check if target is out of range (only for skills with range)
			if (pTarget && m_pSkill->GetRangeType() != CECSkill::RANGE_SLEF && 
				m_pSkill->GetRangeType() != CECSkill::RANGE_SELFSPHERE)
			{
				float fCastRange = m_pSkill->GetCastRange(m_pHost->m_ExtProps.ak.attack_range, m_pHost->GetPrayDistancePlus());
				if (fCastRange > 0.0f)
				{
					float fTargetRad = 0.0f;
					if (ISNPCID(m_idTarget))
					{
						CECNPC* pNPC = dynamic_cast<CECNPC*>(pTarget);
						if (pNPC)
							fTargetRad = pNPC->GetTouchRadius();
					}
					else if (ISPLAYERID(m_idTarget))
					{
						CECElsePlayer* pPlayer = dynamic_cast<CECElsePlayer*>(pTarget);
						if (pPlayer)
							fTargetRad = pPlayer->GetTouchRadius();
					}
					
					// Check if target is out of range
					if (fDist - fTargetRad > fCastRange)
					{
						m_bFinished = true;
						return true;
					}
				}
			}
			
			// Check if target is still valid for casting (attackable/blessable)
			// Only check for attack/curse skills, bless skills might have different validation
			if (m_pSkill->GetType() == CECSkill::TYPE_ATTACK || m_pSkill->GetType() == CECSkill::TYPE_CURSE)
			{
				if (m_pHost->AttackableJudge(m_idTarget, false) != 1)
				{
					m_bFinished = true;
					return true;
				}
			}
		}
		else
		{
			// Target not found (out of sight or destroyed)
			m_bFinished = true;
			return true;
		}
	}

	if (m_idTarget){
		m_pHost->TurnFaceTo(m_idTarget);
	}	
	if (!m_pHost->IsPlayingAction()){
		m_pHost->PlayAction(CECPlayer::ACT_FIGHTSTAND);
	}

	return true;
}




///////////////////////////////////////////////////////////////////////////
//	
//	Implement CECHPWorkSkillStateAction
//	
///////////////////////////////////////////////////////////////////////////

CECHPWorkSkillStateAction::CECHPWorkSkillStateAction(CECHPWorkMan* pWorkMan) :
CECHPWork(WORK_SKILLSTATEACT, pWorkMan)
{
	m_dwMask		= MASK_SKILLSTATEACT;
	m_dwTransMask	= MASK_STAND | MASK_MOVETOPOS | MASK_TRACEOBJECT;
	m_OverTimeCnt.SetCounter(0);
	m_OverTimeCnt.SetPeriod(120000); // 2·ÖÖÓ

	CECHPWorkSkillStateAction::Reset();
}

CECHPWorkSkillStateAction::~CECHPWorkSkillStateAction()
{
}

//	Reset work
void CECHPWorkSkillStateAction::Reset()
{
	CECHPWork::Reset();

	m_OverTimeCnt.Reset();

	m_skill = 0;
}

//	Copy work data
bool CECHPWorkSkillStateAction::CopyData(CECHPWork* pWork)
{
	if (!CECHPWork::CopyData(pWork))
		return false;

	CECHPWorkSkillStateAction* pSrc = (CECHPWorkSkillStateAction*)pWork;

	m_skill	= pSrc->m_skill;

	m_OverTimeCnt.SetPeriod(pSrc->m_OverTimeCnt.GetPeriod());
	m_OverTimeCnt.SetCounter(pSrc->m_OverTimeCnt.GetCounter());
	
	return true;
}

//	On first tick
void CECHPWorkSkillStateAction::OnFirstTick()
{
	CECModel* pModel = m_pHost->GetPlayerModel();
	int state_id = m_pHost->HasSkillStateForAction();
	if(pModel && state_id)
	{
		CECAttacksMan* pAtkMan = g_pGame->GetGameRun()->GetWorld()->GetAttacksMan();
		AString name1,name2;
		if (pAtkMan && pAtkMan->GetSkillStateActionName(m_skill,state_id,name1,name2))
		{
			if (m_pHost->PlayNonSkillActionWithName(CECPlayer::ACT_WOUNDED, name1, false, 0)){
				m_pHost->QueueNonSkillActionWithName(CECPlayer::ACT_WOUNDED, name2, 0);
			}
		}
	}
}

void CECHPWorkSkillStateAction::Cancel()
{
	CECHPWork::Cancel();
}

//	Tick routine
bool CECHPWorkSkillStateAction::Tick(DWORD dwDeltaTime)
{
	CECHPWork::Tick(dwDeltaTime);

	if (m_OverTimeCnt.IncCounter(dwDeltaTime))
			m_bFinished = true;


	return true;
}

