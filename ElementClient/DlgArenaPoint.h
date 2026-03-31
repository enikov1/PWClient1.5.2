// Filename: DlgArenaPoint.h
// Arena of Aurora - Battle point dialog (score display)

#pragma once

#include "DlgBase.h"
#include "AUIStillImageButton.h"
#include "AUILabel.h"
#include "hashmap.h"

class CDlgEnemyTeamMate;
class CECElsePlayer;

// Structure to store enemy player properties from server
struct ArenaEnemyProp
{
	int roleid;
	short level;
	bool combat_state;
	short cls;  // profession
	int hp;
	int max_hp;
	int mp;
	int max_mp;
	
	ArenaEnemyProp() : roleid(0), level(0), combat_state(false), cls(0), 
		hp(0), max_hp(1), mp(0), max_mp(1) {}
};

class CDlgArenaPoint : public CDlgBase
{
	AUI_DECLARE_COMMAND_MAP()

public:
	CDlgArenaPoint();
	virtual ~CDlgArenaPoint();

	virtual bool OnInitDialog();
	virtual void OnTick();

	PAUILABEL m_pTxt_red;
	PAUILABEL m_pTxt_blue;
	PAUILABEL m_pTxt_time;

	int m_iEndBattleTimestamp;
	int m_nInitialTime;
	int m_nTimeRest;  // Time remaining from server (can be negative before battle starts)
	int m_nRedTeamCount;   // Number of alive players in red team
	int m_nBlueTeamCount;  // Number of alive players in blue team
	int m_iSavedBattleCamp;  // Saved battle camp (1=red, 2=blue) to use after battle ends

	bool UpdateEnemyTeamInfo();
	void SetData(int state);
	void UpdateEnemyProp(int roleid, short level, bool combat_state, short cls, int hp, int max_hp, int mp, int max_mp);
	void UpdateTeamCounts(int redCount, int blueCount);

	void OnCommandMinimize(const char* szCommand);
	void OnCommandCANCAL(const char* szCommand);
	void HideEnemiesDlg();

	CDlgEnemyTeamMate* GetEnemyTeamMateDlg(int index);

	abase::hash_map<int, CECElsePlayer*> m_pEnemyList;
	abase::hash_map<int, ArenaEnemyProp> m_EnemyProps;  // Cached enemy properties from server
	abase::vector<CDlgEnemyTeamMate*> m_pEnemyMates;

	enum
	{
		TEAM_STATUS_OPEN = 0,
		TEAM_STATUS_CLOSE
	};

	enum
	{
		ARENA_PREPARE,      // Battle preparation
		ARENA_PREPARE_1,    // Battle preparation 1
		ARENA_ROUND_FIGHT,  // Round in progress
		ARENA_ROUND_REST,   // Round rest period
		ARENA_FINISH,       // Battle finished
	};

	inline int GetStageTime(int state)
	{
		switch (state)
		{
		case ARENA_PREPARE:
			return 75;
		case ARENA_PREPARE_1:
			return 10;
		case ARENA_ROUND_FIGHT:
			return 300;
		case ARENA_ROUND_REST:
			return 40;
		case ARENA_FINISH:
			return 45;
		}
		return 0;
	}
};

