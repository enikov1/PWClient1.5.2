/*
 * Virtual package that maps a game subfolder (e.g. configs\) to loose files on disk.
 * Used when the .pck is missing but an unpacked directory exists — no encryption keys.
 */

#ifndef _AFILEPACKDISK_H_
#define _AFILEPACKDISK_H_

#include "AFilePackBase.h"
#include "AFilePackage.h"

class AFilePackDisk : public AFilePackBase
{
private:
	struct DISK_SHAREDFILE
	{
		bool	bTempMem;
		BYTE*	pFileData;
		DWORD	dwFileLen;
	};

	char	m_szFolder[MAX_PATH];

public:
	AFilePackDisk();
	virtual ~AFilePackDisk();

	// szFolderVirtualPath: same as g_szPckDir entry, e.g. "configs" or "help\\script"
	bool Open(const char* szFolderVirtualPath);
	virtual bool Close();

	virtual const char* GetFolder() { return m_szFolder; }
	virtual bool IsFileExist(const char* szFileName);
	virtual DWORD OpenSharedFile(const char* szFileName, BYTE** ppFileBuf, DWORD* pdwFileLen, bool bTempMem);
	virtual void CloseSharedFile(DWORD dwFileHandle);
};

#endif
