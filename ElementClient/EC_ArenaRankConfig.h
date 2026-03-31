// Filename: EC_ArenaRankConfig.h
// Arena of Aurora - Rank configuration reader
// Reads rank score thresholds from Configs\arenaofaurora.conf

#pragma once

class EC_ArenaRankConfig
{
public:
	static const int RANK_MAX = 16;

	struct RankRange
	{
		int min_score;
		int max_score;
	};

private:
	RankRange m_Ranks[RANK_MAX];
	bool m_bLoaded;

	static EC_ArenaRankConfig* s_pInstance;

public:
	EC_ArenaRankConfig();
	~EC_ArenaRankConfig();

	// Load config from file
	bool Load(const char* szFile = "Configs\\arenaofaurora.conf");

	// Get rank by score (returns 0-15)
	int GetRankByScore(int score) const;

	// Get singleton instance
	static EC_ArenaRankConfig* GetInstance();
	static void Release();
};

