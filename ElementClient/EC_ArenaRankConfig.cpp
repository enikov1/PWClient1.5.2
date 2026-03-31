// Filename: EC_ArenaRankConfig.cpp
// Arena of Aurora - Rank configuration reader

#include "EC_ArenaRankConfig.h"
#include "AIniFile.h"
#include "ALog.h"
#include <cstdio>

EC_ArenaRankConfig* EC_ArenaRankConfig::s_pInstance = NULL;

EC_ArenaRankConfig::EC_ArenaRankConfig()
{
	m_bLoaded = false;
	
	// Initialize with default values matching server config
	const int defaults[RANK_MAX][2] = {
		{0, 999}, {1000, 1199}, {1200, 1399}, {1400, 1599},
		{1600, 1799}, {1800, 1999}, {2000, 2199}, {2200, 2399},
		{2400, 2599}, {2600, 2799}, {2800, 2999}, {3000, 3199},
		{3200, 3399}, {3400, 3599}, {3600, 3799}, {3800, 99999}
	};
	
	for (int i = 0; i < RANK_MAX; i++)
	{
		m_Ranks[i].min_score = defaults[i][0];
		m_Ranks[i].max_score = defaults[i][1];
	}
}

EC_ArenaRankConfig::~EC_ArenaRankConfig()
{
}

EC_ArenaRankConfig* EC_ArenaRankConfig::GetInstance()
{
	if (!s_pInstance)
	{
		s_pInstance = new EC_ArenaRankConfig();
		s_pInstance->Load();
	}
	return s_pInstance;
}

void EC_ArenaRankConfig::Release()
{
	if (s_pInstance)
	{
		delete s_pInstance;
		s_pInstance = NULL;
	}
}

bool EC_ArenaRankConfig::Load(const char* szFile)
{
	AIniFile IniFile;
	
	if (!IniFile.Open(szFile))
		return false;
	
	// Read ranking section
	const char* szSect = "ranking";
	char szKey[32];
	
	for (int i = 0; i < RANK_MAX; i++)
	{
		sprintf(szKey, "rank_%d", i);
		AString strValue = IniFile.GetValueAsString(szSect, szKey, "");
		
		if (!strValue.IsEmpty())
		{
			int min_s = 0, max_s = 0;
			if (sscanf(strValue, "%d, %d", &min_s, &max_s) == 2)
			{
				m_Ranks[i].min_score = min_s;
				m_Ranks[i].max_score = max_s;
			}
		}
	}
	
	IniFile.Close();
	m_bLoaded = true;
	return true;
}

int EC_ArenaRankConfig::GetRankByScore(int score) const
{
	for (int i = 0; i < RANK_MAX; i++)
	{
		if (score >= m_Ranks[i].min_score && score <= m_Ranks[i].max_score)
			return i;
	}
	return 0; // Default to rank 0
}

