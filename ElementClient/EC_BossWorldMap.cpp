#include "EC_BossWorldMap.h"
#include "EC_Global.h"
#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EC_World.h"
#include "EC_ManNPC.h"
#include "EC_NPC.h"
#include "Network/EC_GPDataType.h"
#include "A3DFlatCollector.h"
#include "A3DEngine.h"
#include "A2DSprite.h"
#include "elementdataman.h"
#include "ExpTypes.h"
#include "AFile.h"
#include "AAssist.h"
#include "A3DMacros.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

EC_BossWorldMap* EC_BossWorldMap::s_pInstance = NULL;

EC_BossWorldMap::EC_BossWorldMap()
	: m_bIconTableLoaded(false)
	, m_bShowRespawnGhost(false)
{
	// ASCII defaults; can be overridden by configs\world_map_lg.txt
	m_strRespawnDHM = _AL("Respawn in: %d d %d h %d m");
	m_strRespawnHM  = _AL("Respawn in: %d h %d m");
	m_strRespawnM   = _AL("Respawn in: %d m");
}

EC_BossWorldMap::~EC_BossWorldMap()
{
	ReleaseIconSprites();
}

EC_BossWorldMap* EC_BossWorldMap::GetInstance()
{
	if (!s_pInstance)
		s_pInstance = new EC_BossWorldMap();
	return s_pInstance;
}

void EC_BossWorldMap::ReleaseIconSprites()
{
	for (abase::hash_map<int, A2DSprite*, abase::_hash_function>::iterator it = m_IconSprites.begin();
	     it != m_IconSprites.end();
	     ++it)
		A3DRELEASE(it->second);
	m_IconSprites.clear();
	m_IconLoadFailed.clear();
}

void EC_BossWorldMap::LoadIconConfig()
{
	if (m_bIconTableLoaded)
		return;
	m_bIconTableLoaded = true;

	m_TidToIconPath.clear();
	m_bShowRespawnGhost = false;
	ReleaseIconSprites();

	// Optional localized respawn strings: bin\configs\world_map_lg.txt
	// Line 1: for d/h/m   (e.g. "Появится через: %d дн. %d ч. %d мин.")
	// Line 2: for h/m     (e.g. "Появится через: %d ч. %d мин.")
	// Line 3: for m       (e.g. "Появится через: %d мин.")
	{
		AFile lg;
		if (lg.Open("configs\\world_map_lg.txt", AFILE_OPENEXIST | AFILE_TEXT) ||
		    lg.Open("Configs\\world_map_lg.txt", AFILE_OPENEXIST | AFILE_TEXT))
		{
			char buf[512];
			DWORD rd = 0;
			int lineIdx = 0;
			while (lineIdx < 3 && lg.ReadLine(buf, sizeof(buf) - 1, &rd))
			{
				if (rd == 0)
					continue;
				buf[rd] = '\0';
				char* p = buf;
				while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
					p++;
				if (!*p || *p == '#' || *p == ';')
					continue;
				char* end = p + strlen(p);
				while (end > p && (end[-1] == '\r' || end[-1] == '\n' || end[-1] == ' ' || end[-1] == '\t'))
					*--end = '\0';
				if (end <= p)
					continue;

				switch (lineIdx)
				{
				case 0: m_strRespawnDHM.Format(_AL("%s"), AS2AC(p)); break;
				case 1: m_strRespawnHM .Format(_AL("%s"), AS2AC(p)); break;
				case 2: m_strRespawnM  .Format(_AL("%s"), AS2AC(p)); break;
				}
				lineIdx++;
			}
			lg.Close();
		}
	}

	AFile file;
	if (!file.Open("Configs\\boss_worldmap_icons.conf", AFILE_OPENEXIST | AFILE_TEXT))
		return;

	char line[320];
	DWORD dwRead = 0;
	while (file.ReadLine(line, sizeof(line) - 1, &dwRead))
	{
		if (dwRead == 0)
			continue;
		line[dwRead] = '\0';
		char* p = line;
		while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
			p++;
		if (!*p || *p == '#' || *p == ';')
			continue;
		if (!strncmp(p, "show_respawn_ghost", 18))
		{
			char* v = p + 18;
			while (*v == ' ' || *v == '\t')
				v++;
			m_bShowRespawnGhost = (atoi(v) != 0);
			continue;
		}

		int tid = atoi(p);
		if (tid <= 0)
			continue;
		while (*p && *p != ' ' && *p != '\t')
			p++;
		while (*p == ' ' || *p == '\t')
			p++;
		if (!*p)
			continue;

		char* end = p + strlen(p);
		while (end > p && (end[-1] == '\r' || end[-1] == '\n' || end[-1] == ' ' || end[-1] == '\t'))
			*--end = '\0';
		if (end > p)
			m_TidToIconPath[tid] = p;
	}
	file.Close();
}

A2DSprite* EC_BossWorldMap::GetIconSprite(A3DDevice* pDev, int tid)
{
	if (!pDev)
		return NULL;

	if (m_IconLoadFailed.find(tid) != m_IconLoadFailed.end())
		return NULL;

	abase::hash_map<int, A2DSprite*, abase::_hash_function>::iterator it = m_IconSprites.find(tid);
	if (it != m_IconSprites.end())
		return it->second;

	abase::hash_map<int, AString, abase::_hash_function>::iterator pit = m_TidToIconPath.find(tid);
	if (pit == m_TidToIconPath.end())
		return NULL;

	const char* iconPath = pit->second;
	A2DSprite* spr = new A2DSprite;
	bool ok = (spr && spr->Init(pDev, iconPath, 0));
	if (!ok)
	{
		A3DRELEASE(spr);
		// Fallbacks:
		// 1) case mismatches: Configs\... <-> configs\...
		// 2) missing Surfaces\ prefix (engine often expects Surfaces\...)
		char altPath[512];
		altPath[0] = '\0';
		if (!_strnicmp(iconPath, "Configs\\", 8))
			_snprintf(altPath, sizeof(altPath) - 1, "configs\\%s", iconPath + 8);
		else if (!_strnicmp(iconPath, "configs\\", 8))
			_snprintf(altPath, sizeof(altPath) - 1, "Configs\\%s", iconPath + 8);
		else
			_snprintf(altPath, sizeof(altPath) - 1, "Configs\\%s", iconPath);
		altPath[sizeof(altPath) - 1] = '\0';
		if (altPath[0])
		{
			spr = new A2DSprite;
			ok = (spr && spr->Init(pDev, altPath, 0));
		}
	}
	if (!ok)
	{
		// Last try: Surfaces\ prefix (where textures usually live)
		char surfPath[600];
		_snprintf(surfPath, sizeof(surfPath) - 1, "Surfaces\\%s", iconPath);
		surfPath[sizeof(surfPath) - 1] = '\0';
		spr = new A2DSprite;
		ok = (spr && spr->Init(pDev, surfPath, 0));
		if (!ok)
		{
			A3DRELEASE(spr);
			m_IconLoadFailed[tid] = 1;
			return NULL;
		}
	}
	spr->SetLinearFilter(true);
	m_IconSprites[tid] = spr;
	return spr;
}

int EC_BossWorldMap::GetEssenceBaseLife(int tid) const
{
	if (!g_pGame || !g_pGame->GetElementDataMan())
		return 0;
	DATA_TYPE dt = DT_INVALID;
	const void* raw = g_pGame->GetElementDataMan()->get_data_ptr(tid, ID_SPACE_ESSENCE, dt);
	if (raw && dt == DT_MONSTER_ESSENCE)
		return ((const MONSTER_ESSENCE*)raw)->life;
	return 0;
}

void EC_BossWorldMap::ClearWorldState(int worldTag)
{
	abase::vector<int> rm;
	for (abase::hash_map<int, ServerSnap, abase::_hash_function>::iterator it = m_ServerByKey.begin();
	     it != m_ServerByKey.end();
	     ++it)
	{
		if (it->second.worldId == worldTag)
			rm.push_back(it->first);
	}
	for (size_t j = 0; j < rm.size(); j++)
	{
		int k = rm[j];
		int tid = m_ServerByKey[k].tid;
		m_ServerByKey.erase(k);
		abase::hash_map<int, A2DSprite*, abase::_hash_function>::iterator sit = m_IconSprites.find(tid);
		if (sit != m_IconSprites.end())
		{
			A3DRELEASE(sit->second);
			m_IconSprites.erase(sit);
		}
		m_IconLoadFailed.erase(tid);
	}
}

ACString EC_BossWorldMap::FormatMonsterName(int tid) const
{
	if (!g_pGame || !g_pGame->GetElementDataMan())
		return ACString().Format(_AL("boss %d"), tid);

	DATA_TYPE dt = DT_INVALID;
	const void* p = g_pGame->GetElementDataMan()->get_data_ptr(tid, ID_SPACE_ESSENCE, dt);
	if (p && dt == DT_MONSTER_ESSENCE)
		return ACString(((const MONSTER_ESSENCE*)p)->name);
	if (p && dt == DT_NPC_ESSENCE)
		return ACString(((const NPC_ESSENCE*)p)->name);
	return ACString().Format(_AL("boss %d"), tid);
}

void EC_BossWorldMap::ApplyServerSync(const S2C::cmd_boss_world_map_sync* p, size_t bufSize)
{
	if (!p)
		return;
	size_t sz = 0;
	if (!p->CheckValid(bufSize, sz))
		return;

	LoadIconConfig();

	const unsigned int cnt = p->count;
	const size_t stride = (cnt == 0)
		? 0u
		: ((bufSize - sizeof(unsigned int)) / (size_t)cnt);

	if (cnt > 0)
	{
		const BYTE* row0 = (const BYTE*)p + sizeof(unsigned int);
		ClearWorldState(reinterpret_cast<const int*>(row0)[1]);
	}

	for (unsigned int i = 0; i < cnt; i++)
	{
		const BYTE* row = (const BYTE*)p + sizeof(unsigned int) + (size_t)i * stride;
		const int* pi = reinterpret_cast<const int*>(row);
		const float* pf = reinterpret_cast<const float*>(row + 12);
		int tid = pi[0];
		int world_tag = pi[1];
		int alive = pi[2];
		float x = pf[0];
		float z = pf[1];
		int rem = pi[5];
		int curHp = 0;
		int maxHp = 0;
		if (stride >= 32)
		{
			curHp = pi[6];
			maxHp = pi[7];
		}
		int k = MakeKey(world_tag, tid);

		ServerSnap snap;
		snap.valid = true;
		snap.tid = tid;
		snap.worldId = world_tag;
		snap.alive = (alive != 0);
		snap.x = x;
		snap.z = z;
		snap.curHp = curHp;
		snap.maxHp = maxHp;
		DWORD now = GetTickCount();
		if (snap.alive)
			snap.endTick = now;
		else
		{
			if (rem < 0)
				rem = 0;
			snap.endTick = now + (DWORD)rem * 1000;
		}
		m_ServerByKey[k] = snap;
	}
}

static void FillCircle2D(A3DFlatCollector* pFC, float cx, float cy, float r, DWORD col)
{
	const int N = 22;
	A3DVECTOR3 verts[N + 1];
	WORD idx[N * 3];
	verts[0] = A3DVECTOR3(cx, cy, 0.f);
	for (int i = 0; i < N; i++)
	{
		float a = (float)(2.0 * M_PI * i / N);
		verts[i + 1] = A3DVECTOR3(cx + cosf(a) * r, cy + sinf(a) * r, 0.f);
	}
	int ni = 0;
	for (int i = 0; i < N; i++)
	{
		idx[ni++] = 0;
		idx[ni++] = (WORD)(1 + i);
		idx[ni++] = (WORD)(1 + ((i + 1) % N));
	}
	pFC->AddRenderData_2D(verts, N + 1, idx, ni, col);
}

void EC_BossWorldMap::DrawBossDot(A3DFlatCollector* pFC, float cx, float cy, bool alive)
{
	DWORD edge = alive ? A3DCOLORRGBA(120, 45, 0, 255) : A3DCOLORRGBA(80, 80, 80, 255);
	DWORD fill = alive ? A3DCOLORRGBA(255, 210, 70, 255) : A3DCOLORRGBA(190, 190, 190, 255);
	FillCircle2D(pFC, cx, cy, 9.5f, edge);
	FillCircle2D(pFC, cx, cy, 6.5f, fill);
	FillCircle2D(pFC, cx, cy, 2.2f, alive ? A3DCOLORRGBA(255, 248, 220, 255) : A3DCOLORRGBA(220, 220, 220, 255));
}

void EC_BossWorldMap::Draw(
	const CECMapDlgsMgr::PosConvertor& conv,
	int worldId,
	A3DFlatCollector* pFC,
	A3DDevice* pDevice)
{
	m_HoverPoints.clear();
	if (!pFC || !g_pGame || !g_pGame->GetGameRun() || !g_pGame->GetGameRun()->GetWorld())
		return;

	LoadIconConfig();

	CECNPCMan* pNpcMan = g_pGame->GetGameRun()->GetWorld()->GetNPCMan();
	if (!pNpcMan)
		return;

	abase::vector<A2DSprite*> drawnSprites;
	abase::vector<int> drawnX;
	abase::vector<int> drawnY;

	for (abase::hash_map<int, ServerSnap, abase::_hash_function>::iterator it = m_ServerByKey.begin();
	     it != m_ServerByKey.end();
	     ++it)
	{
		const ServerSnap& srv = it->second;
		if (!srv.valid || srv.worldId != worldId)
			continue;

		const bool srvAlive = (srv.alive != 0);
		int tid = srv.tid;

		// dead boss: if ghost mode выключен — вообще не показываем
		if (!srvAlive && !m_bShowRespawnGhost)
			continue;

		CECNPC* pLive = pNpcMan->FindNPCByTempleID(tid);
		if (pLive && (pLive->IsDead() || pLive->GetBossMapCurHP() <= 0))
			pLive = NULL; // локально мертвый/0 HP — считаем как нет живого NPC

		bool drawAlive = false;
		A3DVECTOR3 wpos(0, 0, 0);
		int remainSec = 0;

		if (srvAlive)
		{
			// Живой по серверу: всегда считаем маркер живым, позиция из NPC если есть, иначе из снапшота.
			drawAlive = true;
			if (pLive)
				wpos = pLive->GetPos();
			else
			{
				wpos.x = srv.x;
				wpos.y = 0.f;
				wpos.z = srv.z;
			}
		}
		else
		{
			// Мертвый по серверу: рисуем "тень" в точке спавна, считаем время до респавна.
			drawAlive = false;
			wpos.x = srv.x;
			wpos.y = 0.f;
			wpos.z = srv.z;

			DWORD now = GetTickCount();
			if (now < srv.endTick)
				remainSec = (int)((srv.endTick - now + 999) / 1000);
		}

		A3DPOINT2 absPos = conv(wpos);
		float cx = (float)absPos.x;
		float cy = (float)absPos.y;

		int dispCur = srv.curHp;
		int dispMax = srv.maxHp;
		if (srvAlive)
		{
			if (pLive)
			{
				dispCur = pLive->GetBossMapCurHP();
				dispMax = pLive->GetBossMapMaxHP();
			}
			if (dispMax <= 0)
				dispMax = GetEssenceBaseLife(tid);
			if (dispCur <= 0 && dispMax > 0)
				dispCur = dispMax;
		}

		A2DSprite* spr = pDevice ? GetIconSprite(pDevice, tid) : NULL;
		int iconHalf = 14;
		if (spr)
		{
			int iw = spr->GetWidth();
			int ih = spr->GetHeight();
			if (ih <= 0)
				ih = 24;
			if (iw <= 0)
				iw = 24;
			int maxDim = a_Max(iw, ih);
			if (maxDim > 36)
			{
				iw = iw * 36 / maxDim;
				ih = ih * 36 / maxDim;
			}
			iconHalf = a_Max(iw, ih) / 2 + 6;
			drawnSprites.push_back(spr);
			drawnX.push_back((int)(cx - iw / 2));
			drawnY.push_back((int)(cy - ih / 2));
		}
		else
			DrawBossDot(pFC, cx, cy, drawAlive);

		HoverPoint hp;
		hp.scrX = cx;
		hp.scrY = cy;
		hp.hitRadius = iconHalf;
		hp.wPos = wpos;
		hp.strHint = FormatMonsterName(tid);

		if (srvAlive && dispMax > 0)
		{
			hp.strHint += ACString().Format(_AL("\nHP: %d / %d"), a_Max(dispCur, 0), dispMax);
		}
		else if (!srvAlive && remainSec > 0)
		{
			int totalMin = remainSec / 60;
			int hours = totalMin / 60;
			int mins = totalMin % 60;
			int days = hours / 24;
			hours = hours % 24;

			if (days > 0)
			{
				const ACString& fmt = m_strRespawnDHM;
				if (!fmt.IsEmpty())
				{
					ACString line;
					line.Format(fmt, days, hours, mins);
					hp.strHint += _AL("\n");
					hp.strHint += line;
				}
			}
			else if (hours > 0)
			{
				const ACString& fmt = m_strRespawnHM;
				if (!fmt.IsEmpty())
				{
					ACString line;
					line.Format(fmt, hours, mins);
					hp.strHint += _AL("\n");
					hp.strHint += line;
				}
			}
			else
			{
				const ACString& fmt = m_strRespawnM;
				if (!fmt.IsEmpty())
				{
					ACString line;
					line.Format(fmt, mins);
					hp.strHint += _AL("\n");
					hp.strHint += line;
				}
			}
		}

		m_HoverPoints.push_back(hp);
	}

	pFC->Flush();

	if (pDevice)
	{
		for (size_t di = 0; di < drawnSprites.size(); di++)
		{
			A2DSprite* s = drawnSprites[di];
			if (s)
				s->DrawToBack((float)drawnX[di], (float)drawnY[di]);
		}
	}
}

bool EC_BossWorldMap::PickHover(int scrX, int scrY, ACString& outHint) const
{
	for (size_t i = 0; i < m_HoverPoints.size(); i++)
	{
		const HoverPoint& h = m_HoverPoints[i];
		int r = h.hitRadius > 0 ? h.hitRadius : 12;
		if (abs(scrX - (int)h.scrX) <= r && abs(scrY - (int)h.scrY) <= r)
		{
			outHint = h.strHint;
			return true;
		}
	}
	return false;
}

bool EC_BossWorldMap::PickNearWorldPos(float worldX, float worldZ, A3DVECTOR3& outDest)
{
	if (!g_pGame || !g_pGame->GetGameRun() || !g_pGame->GetGameRun()->GetWorld())
		return false;

	int worldId = g_pGame->GetGameRun()->GetWorld()->GetInstanceID();
	CECNPCMan* pNpcMan = g_pGame->GetGameRun()->GetWorld()->GetNPCMan();
	if (!pNpcMan)
		return false;

	const float pickR = 70.f;
	const float pickR2 = pickR * pickR;
	float best = pickR2 + 1.f;
	bool found = false;

	for (abase::hash_map<int, ServerSnap, abase::_hash_function>::iterator it = m_ServerByKey.begin();
	     it != m_ServerByKey.end();
	     ++it)
	{
		const ServerSnap& srv = it->second;
		if (!srv.valid || srv.worldId != worldId)
			continue;

		CECNPC* pLive = pNpcMan->FindNPCByTempleID(srv.tid);
		if (pLive && (pLive->IsDead() || pLive->GetBossMapCurHP() <= 0))
			pLive = NULL;
		if (!pLive && !m_bShowRespawnGhost)
			continue; // Default: auto-path only to currently existing boss marker.

		A3DVECTOR3 wpos(0, 0, 0);
		if (pLive)
			wpos = pLive->GetPos();
		else
		{
			wpos.x = srv.x;
			wpos.y = 0.f;
			wpos.z = srv.z;
		}

		float dx = worldX - wpos.x;
		float dz = worldZ - wpos.z;
		float d2 = dx * dx + dz * dz;
		if (d2 <= pickR2 && d2 < best)
		{
			best = d2;
			outDest = wpos;
			found = true;
		}
	}
	return found;
}
