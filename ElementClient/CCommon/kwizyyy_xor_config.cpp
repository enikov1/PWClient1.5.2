#include "kwizyyy_xor_config.h"

#include <string.h>

#include "ALog.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#define KWIZYYY_XOR_INI_REL	"Configs\\element_data_xor.ini"
#define KWIZYYY_XOR_DEV_REL	"Configs\\element_data_xor.dev"
#define KWIZYYY_XOR_SECTION	"DataXor"
#define KWIZYYY_XOR_MAX_KEY	256

static const char s_defaultKey[] = "TROUXA";

struct KwizyyyXorCtx {
	bool initialized;
	bool enabled;
	unsigned char key[KWIZYYY_XOR_MAX_KEY];
	size_t keyLen;
};

static KwizyyyXorCtx g_ctx;

static void KwizyyyXorInit_impl();

static void KwizyyyXorInit()
{
#ifdef _WIN32
	KwizyyyXorInit_impl();
#else
	if (g_ctx.initialized)
		return;
	g_ctx.initialized = true;
	g_ctx.enabled = false;
	g_ctx.keyLen = 0;
#endif
}

#ifdef _WIN32
static bool KwizyyyXorFileExistsA(const char* p)
{
	DWORD a = GetFileAttributesA(p);
	return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static bool KwizyyyXorJoinBaseRel(char* out, const char* base, const char* rel)
{
	size_t bl = strlen(base);
	size_t rl = strlen(rel);
	if (bl + 1 + rl >= (size_t)MAX_PATH)
		return false;
	memcpy(out, base, bl);
	out[bl] = '\\';
	memcpy(out + bl + 1, rel, rl + 1);
	return true;
}

/* rel e.g. Configs\file — cwd, exe folder, parent of exe folder */
static bool KwizyyyXorFindRelPath(char pathOut[MAX_PATH], const char* relFromGameRoot)
{
	char base[MAX_PATH];

	if (GetCurrentDirectoryA(MAX_PATH, base) && base[0] &&
	    KwizyyyXorJoinBaseRel(pathOut, base, relFromGameRoot) && KwizyyyXorFileExistsA(pathOut))
		return true;

	char exeFull[MAX_PATH];
	if (GetModuleFileNameA(NULL, exeFull, MAX_PATH) == 0 || !exeFull[0])
		return false;

	strcpy_s(base, exeFull);
	char* slash = strrchr(base, '\\');
	if (slash)
		*slash = '\0';
	if (!base[0])
		return false;

	if (KwizyyyXorJoinBaseRel(pathOut, base, relFromGameRoot) && KwizyyyXorFileExistsA(pathOut))
		return true;

	slash = strrchr(base, '\\');
	if (slash && slash != base) {
		*slash = '\0';
		return KwizyyyXorJoinBaseRel(pathOut, base, relFromGameRoot) && KwizyyyXorFileExistsA(pathOut);
	}

	return false;
}

/* ini is honored only with unlock (do not ship .dev in public client). */
static bool KwizyyyXorDeveloperUnlocked()
{
#if defined(ALLOW_ELEMENT_XOR_CONFIG)
	return true;
#else
	char unused[MAX_PATH];
	return KwizyyyXorFindRelPath(unused, KWIZYYY_XOR_DEV_REL);
#endif
}

static void KwizyyyXorInit_impl()
{
	if (g_ctx.initialized)
		return;
	g_ctx.initialized = true;
#if defined(KWIZYYY_READ_PLAIN_DATA_FILES)
	g_ctx.enabled = false;
	g_ctx.keyLen = 0;
	a_LogOutput(1, "KwizyyyXor: disabled (plain), KWIZYYY_READ_PLAIN_DATA_FILES");
	return;
#endif
	g_ctx.enabled = true;

	if (!KwizyyyXorDeveloperUnlocked()) {
		memcpy(g_ctx.key, s_defaultKey, sizeof(s_defaultKey) - 1);
		g_ctx.keyLen = sizeof(s_defaultKey) - 1;
		return;
	}

	char path[MAX_PATH];
	if (!KwizyyyXorFindRelPath(path, KWIZYYY_XOR_INI_REL)) {
		memcpy(g_ctx.key, s_defaultKey, sizeof(s_defaultKey) - 1);
		g_ctx.keyLen = sizeof(s_defaultKey) - 1;
		return;
	}

	UINT er = SetErrorMode(SEM_FAILCRITICALERRORS);
	int enabled = GetPrivateProfileIntA(KWIZYYY_XOR_SECTION, "enabled", 1, path);
	SetErrorMode(er);

	char keyStr[KWIZYYY_XOR_MAX_KEY + 1];
	keyStr[0] = '\0';
	GetPrivateProfileStringA(KWIZYYY_XOR_SECTION, "key", "", keyStr, KWIZYYY_XOR_MAX_KEY, path);

	if (!enabled) {
		g_ctx.enabled = false;
		g_ctx.keyLen = 0;
		a_LogOutput(1, "KwizyyyXor: disabled (plain), ini: %s", path);
		return;
	}

	if (keyStr[0] == '\0') {
		memcpy(g_ctx.key, s_defaultKey, sizeof(s_defaultKey) - 1);
		g_ctx.keyLen = sizeof(s_defaultKey) - 1;
	} else {
		g_ctx.keyLen = strlen(keyStr);
		if (g_ctx.keyLen > KWIZYYY_XOR_MAX_KEY)
			g_ctx.keyLen = KWIZYYY_XOR_MAX_KEY;
		memcpy(g_ctx.key, keyStr, g_ctx.keyLen);
	}
}
#endif /* _WIN32 */

void KwizyyyXorApplyToBuffer(char* out, const char* in, size_t len)
{
	KwizyyyXorInit();

	if (!g_ctx.enabled || g_ctx.keyLen == 0) {
		memcpy(out, in, len);
		return;
	}

	for (size_t i = 0; i < len; i++) {
		unsigned char xortarget = g_ctx.key[i % g_ctx.keyLen];
		unsigned char b = (unsigned char)in[i];
		if (b != 0 && b != xortarget)
			out[i] = (char)(b ^ xortarget);
		else
			out[i] = (char)b;
	}
}
