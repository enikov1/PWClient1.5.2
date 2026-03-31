// World map (M): boss markers — данные с GS (BOSS_WORLD_MAP_SYNC).
// Опционально: клиент Configs\boss_worldmap_icons.conf — tid и путь к текстуре.

#pragma once

#include "DlgWorldMap.h"
#include "vector.h"
#include "hashmap.h"
#include "A3DVector.h"
#include "AWString.h"
#include "AString.h"

class A3DFlatCollector;
class A3DDevice;
class A2DSprite;

namespace S2C { struct cmd_boss_world_map_sync; }

class EC_BossWorldMap
{
public:
	struct ServerSnap
	{
		bool valid;
		int tid;
		int worldId;
		bool alive;
		float x, z;
		DWORD endTick;
		int curHp;
		int maxHp;
		ServerSnap()
			: valid(false), tid(0), worldId(0), alive(false), x(0), z(0), endTick(0), curHp(0), maxHp(0) {}
	};

	struct HoverPoint
	{
		float scrX;
		float scrY;
		int hitRadius;
		ACString strHint;
		A3DVECTOR3 wPos;
	};

private:
	abase::hash_map<int, ServerSnap, abase::_hash_function> m_ServerByKey;
	mutable abase::vector<HoverPoint> m_HoverPoints;

	abase::hash_map<int, AString, abase::_hash_function> m_TidToIconPath;
	abase::hash_map<int, A2DSprite*, abase::_hash_function> m_IconSprites;
	abase::hash_map<int, int, abase::_hash_function> m_IconLoadFailed;
	bool m_bIconTableLoaded;
	bool m_bShowRespawnGhost;
	ACString m_strRespawnDHM;
	ACString m_strRespawnHM;
	ACString m_strRespawnM;

	static EC_BossWorldMap* s_pInstance;

	EC_BossWorldMap();
	~EC_BossWorldMap();

	static int MakeKey(int worldId, int tid) { return worldId * 1000000 + tid; }
	ACString FormatMonsterName(int tid) const;
	void ClearWorldState(int worldTag);
	void LoadIconConfig();
	A2DSprite* GetIconSprite(A3DDevice* pDev, int tid);
	void ReleaseIconSprites();
	int GetEssenceBaseLife(int tid) const;
	static void DrawBossDot(A3DFlatCollector* pFC, float cx, float cy, bool alive);

public:
	static EC_BossWorldMap* GetInstance();

	void ApplyServerSync(const S2C::cmd_boss_world_map_sync* p, size_t bufSize);

	void Draw(
		const CECMapDlgsMgr::PosConvertor& conv,
		int worldId,
		A3DFlatCollector* pFC,
		A3DDevice* pDevice);

	const abase::vector<HoverPoint>& GetHoverPoints() const { return m_HoverPoints; }

	bool PickHover(int scrX, int scrY, ACString& outHint) const;
	bool PickNearWorldPos(float worldX, float worldZ, A3DVECTOR3& outDest);
};
