#include "AFilePackDisk.h"
#include "AFI.h"
#include "AFPI.h"
#include "AMemory.h"

#include <Windows.h>

#define new A_DEBUG_NEW

static void NormalizeFolderPath(char* szPath)
{
	if (!szPath || !szPath[0])
		return;

	int len = strlen(szPath);
	for (int i = 0; i < len; ++i)
	{
		if (szPath[i] == '/')
			szPath[i] = '\\';
	}

	for (int i = 0; i < len - 1; )
	{
		if (szPath[i] == '\\' && szPath[i + 1] == '\\')
		{
			for (int j = i; j < len - 1; ++j)
				szPath[j] = szPath[j + 1];
			szPath[len - 1] = '\0';
			--len;
		}
		else
		{
			++i;
		}
	}

	if (len > 2 && szPath[0] == '.' && szPath[1] == '\\')
	{
		for (int i = 0; i < len - 2; ++i)
			szPath[i] = szPath[i + 2];
		szPath[len - 2] = '\0';
	}

	len = strlen(szPath);
	for (int i = len - 1; i >= 0; --i)
	{
		if (szPath[i] != ' ')
			break;
		szPath[i] = '\0';
	}
}

AFilePackDisk::AFilePackDisk()
{
	m_szFolder[0] = '\0';
}

AFilePackDisk::~AFilePackDisk()
{
	Close();
}

bool AFilePackDisk::Open(const char* szFolderVirtualPath)
{
	if (!szFolderVirtualPath || !szFolderVirtualPath[0])
		return false;

	strncpy(m_szFolder, szFolderVirtualPath, MAX_PATH);
	m_szFolder[MAX_PATH - 1] = '\0';
	strlwr(m_szFolder);
	NormalizeFolderPath(m_szFolder);

	int len = strlen(m_szFolder);
	while (len > 1 && m_szFolder[len - 1] == '\\')
	{
		m_szFolder[len - 1] = '\0';
		len--;
	}

	if (len <= 0)
		return false;

	m_szFolder[len] = '\\';
	m_szFolder[len + 1] = '\0';

	char szOnDisk[MAX_PATH];
	char szTrimFolder[MAX_PATH];
	strncpy(szTrimFolder, m_szFolder, MAX_PATH);
	szTrimFolder[MAX_PATH - 1] = '\0';
	int tl = strlen(szTrimFolder);
	if (tl > 0 && szTrimFolder[tl - 1] == '\\')
		szTrimFolder[tl - 1] = '\0';

	af_GetFullPath(szOnDisk, szTrimFolder);

	DWORD dwAttr = GetFileAttributesA(szOnDisk);
	if (dwAttr == INVALID_FILE_ATTRIBUTES || !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		return false;

	return true;
}

bool AFilePackDisk::Close()
{
	m_szFolder[0] = '\0';
	return true;
}

bool AFilePackDisk::IsFileExist(const char* szFileName)
{
	if (!szFileName || !szFileName[0])
		return false;

	char szFull[MAX_PATH];
	af_GetFullPath(szFull, szFileName);

	DWORD dwAttr = GetFileAttributesA(szFull);
	if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		return false;

	return true;
}

DWORD AFilePackDisk::OpenSharedFile(const char* szFileName, BYTE** ppFileBuf, DWORD* pdwFileLen, bool bTempMem)
{
	if (!IsFileExist(szFileName))
		return 0;

	char szFull[MAX_PATH];
	af_GetFullPath(szFull, szFileName);

	FILE* fp = fopen(szFull, "rb");
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	long nLen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (nLen <= 0)
	{
		fclose(fp);
		return 0;
	}

	BYTE* pFileData = NULL;
	if (bTempMem)
		pFileData = (BYTE*)a_malloctemp((DWORD)nLen);
	else
		pFileData = (BYTE*)a_malloc((DWORD)nLen);

	if (!pFileData)
	{
		fclose(fp);
		return 0;
	}

	if (fread(pFileData, 1, (size_t)nLen, fp) != (size_t)nLen)
	{
		fclose(fp);
		if (bTempMem)
			a_freetemp(pFileData);
		else
			a_free(pFileData);
		return 0;
	}
	fclose(fp);

	DISK_SHAREDFILE* pItem = new DISK_SHAREDFILE;
	if (!pItem)
	{
		if (bTempMem)
			a_freetemp(pFileData);
		else
			a_free(pFileData);
		return 0;
	}

	pItem->bTempMem = bTempMem;
	pItem->pFileData = pFileData;
	pItem->dwFileLen = (DWORD)nLen;

	*ppFileBuf = pFileData;
	*pdwFileLen = pItem->dwFileLen;

	return (DWORD)pItem;
}

void AFilePackDisk::CloseSharedFile(DWORD dwFileHandle)
{
	DISK_SHAREDFILE* pItem = (DISK_SHAREDFILE*)dwFileHandle;
	if (!pItem)
		return;

	if (pItem->bTempMem)
		a_freetemp(pItem->pFileData);
	else
		a_free(pItem->pFileData);

	delete pItem;
}
