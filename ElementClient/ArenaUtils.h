// Filename: ArenaUtils.h
// Arena of Aurora - Utility functions for team mapping and conversions

#pragma once

#include "Network/EC_GPDataType.h"

// Server-side constants
#define ARENA_RED_TYPE_IDX    2048
#define ARENA_BLUE_TYPE_IDX   4096
#define ARENA_RESULT_WINNER_RED   1
#define ARENA_RESULT_WINNER_BLUE  2
#define ARENA_RESULT_NOBODY       0

// Client-side constants (from EC_GPDataType.h)
// GP_BATTLE_CAMP_NONE = 0
// GP_BATTLE_CAMP_INVADER = 1
// GP_BATTLE_CAMP_DEFENDER = 2

namespace ArenaUtils
{
    // Convert server team_id to client battle camp
    // team_id: RED_TYPE_IDX (2048) or BLUE_TYPE_IDX (4096)
    // Returns: GP_BATTLE_CAMP_INVADER or GP_BATTLE_CAMP_DEFENDER
    inline int TeamIdToBattleCamp(unsigned int team_id)
    {
        if (team_id == ARENA_RED_TYPE_IDX)
            return GP_BATTLE_CAMP_INVADER;  // Red team = Invader
        else if (team_id == ARENA_BLUE_TYPE_IDX)
            return GP_BATTLE_CAMP_DEFENDER;  // Blue team = Defender
        return GP_BATTLE_CAMP_NONE;
    }

    // Convert client battle camp to server team_id
    // battle_camp: GP_BATTLE_CAMP_INVADER or GP_BATTLE_CAMP_DEFENDER
    // Returns: RED_TYPE_IDX or BLUE_TYPE_IDX
    inline unsigned int BattleCampToTeamId(int battle_camp)
    {
        if (battle_camp == GP_BATTLE_CAMP_INVADER)
            return ARENA_RED_TYPE_IDX;
        else if (battle_camp == GP_BATTLE_CAMP_DEFENDER)
            return ARENA_BLUE_TYPE_IDX;
        return 0;
    }

    // Check if team_id is red team
    inline bool IsRedTeam(unsigned int team_id)
    {
        return team_id == ARENA_RED_TYPE_IDX;
    }

    // Check if team_id is blue team
    inline bool IsBlueTeam(unsigned int team_id)
    {
        return team_id == ARENA_BLUE_TYPE_IDX;
    }

    // Convert team_win value to winner team_id
    // team_win: 1 = red won, 2 = blue won, 0 = draw
    // Returns: RED_TYPE_IDX, BLUE_TYPE_IDX, or 0 for draw
    inline unsigned int TeamWinToTeamId(int team_win)
    {
        if (team_win == ARENA_RESULT_WINNER_RED)
            return ARENA_RED_TYPE_IDX;
        else if (team_win == ARENA_RESULT_WINNER_BLUE)
            return ARENA_BLUE_TYPE_IDX;
        return 0;  // Draw
    }

    // Check if player's team won
    // team_win: 1 = red won, 2 = blue won
    // player_team_id: RED_TYPE_IDX or BLUE_TYPE_IDX
    // Returns: true if player's team won
    inline bool IsPlayerTeamWinner(int team_win, unsigned int player_team_id)
    {
        if (team_win == ARENA_RESULT_WINNER_RED && player_team_id == ARENA_RED_TYPE_IDX)
            return true;
        if (team_win == ARENA_RESULT_WINNER_BLUE && player_team_id == ARENA_BLUE_TYPE_IDX)
            return true;
        return false;
    }

    // Get opponent team_id
    inline unsigned int GetOpponentTeamId(unsigned int team_id)
    {
        if (team_id == ARENA_RED_TYPE_IDX)
            return ARENA_BLUE_TYPE_IDX;
        else if (team_id == ARENA_BLUE_TYPE_IDX)
            return ARENA_RED_TYPE_IDX;
        return 0;
    }
}

