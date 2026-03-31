/*
 * FILE: EC_Configs.cpp
 *
 * DESCRIPTION: 
 *
 * CREATED BY: Duyuxin, 2004/8/27
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#include "EC_Global.h"
#include "EC_Configs.h"
#include "EC_World.h"
#include "EC_Game.h"
#include "EC_GameRun.h"
#include "EL_Forest.h"
#include "EL_GrassLand.h"
#include "EC_NPC.h"
#include "EC_ShadowRender.h"
#include "EC_FullGlowRender.h"
#include "EC_Viewport.h"
#include "EC_SceneLoader.h"
#include "EC_SceneBlock.h"
#include "ELTerrainOutline2.h"
#include "EL_CloudManager.h"
#include "EC_ManOrnament.h"
#include "EC_HostPlayer.h"
#include "EC_Skill.h"
#include "EC_RTDebug.h"
#include "EC_Utility.h"
#include "EC_GameSession.h"
#include "EC_UIManager.h"
#include "EC_GameUIMan.h"
#include "DlgSettingGame.h"
#include "EC_CommandLine.h"
#include "EC_CrossServer.h"
#include "EC_Optimize.h"
#include "EC_MemSimplify.h"
#include "EC_UIConfigs.h"
#include <CSplit.h>

#include "AIniFile.h"
#include "AWIniFile.h"
#include "A3DMacros.h"
#include "A3DTerrainWater.h"
#include "A3DTerrain2.h"
#include "AAssist.h"
#include "A3DEngine.h"
#include "A3DDevice.h"
#include "A3DViewport.h"
#include "A3DCameraBase.h"
#include "A3DCamera.h"
#include "A3DGfxExMan.h"
#include "A3DConfig.h"
#include "AMConfig.h"
#include "AMEngine.h"
#include "AMSoundEngine.h"
#include "AMSoundBufferMan.h"
#include "AFI.h"

#define new A_DEBUG_NEW

///////////////////////////////////////////////////////////////////////////
//	
//	Define and Macro
//	
///////////////////////////////////////////////////////////////////////////

//	Configs data version
//#define EC_CONFIG_VERSION	37	// 1.5.3
#define EC_CONFIG_VERSION	42  // Added TAB selection radius and class filter
// v26 增加战场频道
// v27 增加自动喝药参数
// v28 增加女王卡片提醒参数
// v29 增加国家频道
// v30 自动喝药参数扩充
// v31 关闭绝圣冲击提示
// v32 增加锁定快捷栏，宠物自动增益
// v33 增加任务自动组队选项
// v34 增加禁用智能百科选项
// v35 增加专属奖励显示模式
// v36 增加隐藏月仙冰雷球
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
//	Implement CECConfigs
//	
///////////////////////////////////////////////////////////////////////////

CECConfigs::CECConfigs()
{
	//	Configs user cannot change ...
	m_iCodePage			= 936;
	m_fSceLoadRadius	= 500.0f;
	m_dwDefFogColor		= A3DCOLORRGB(70, 77, 91);
	m_fSevActiveRad		= 213.0f;	//	能正确处理服务器端 slice 最大是 50 的情况
	m_fForestDetail		= 1.0f;
	m_fGrassLandDetail	= 1.0f;
	m_fTrnLODDist1		= 180.0f;
	m_fTrnLODDist2		= 320.0f;
	m_dwPlayerText		= 0;
	m_fMinPVRadius		= 5.0f;
	m_fCurPVRadius		= m_fSevActiveRad;
	m_fPlayerNameRenderRadius = 60.0f;	// Default player name render radius
	m_fNPCNameRenderRadius = 20.0f;		// Default NPC name render radius
	m_fModelRenderRadius = 60.0f;		// Default model render radius
	m_iMaxNameLen		= 9;
	m_iMultiClient		= 1;
	m_strWindowsTitle	= _AL("Element Client");
	m_iClientID	= -1;
	m_bRegisterUIScriptFunc = false;
	m_bEnableGT = false;
	m_bEnableArc = false;
	m_bMiniClient		= false;
	m_bSendLogicCheckInfo	= false;

	//	DEBUG only settings ...
	m_bConsole			= false;
	m_iRTDebug			= 1;
	m_bShowNPCID		= false;
	m_fRunSpeed			= 5.0f;
	m_fWalkSpeed		= 2.0f;
	m_bTestDist			= false;
	m_bShowPos			= false;
	m_bShowForest		= true;
	m_bGameStat			= false;
	m_bShowID			= false;
	m_bSkipFrame		= false;
	m_bModelUpt			= true;
	m_bDebugFog			= false;
	m_fDebugFogStart	= 10.0f;
	m_fDebugFogEnd		= 100.0f;
	m_fDebugFogDensity	= 0.5f;
	m_dwDebugFogColor	= A3DCOLORRGB(255, 255, 255);

	m_bAdjusting		= false;

	//	User settings ...
	DefaultUserSettings(&m_ss, &m_vs, &m_gs, &m_bs, &m_cas, &m_uis);

	//	Test code ... 
	m_bShowForest		= true;
	m_bShowGrassLand	= true;
}

CECConfigs::~CECConfigs()
{
}

//	Initialize object
bool CECConfigs::Init(const char* szCfgFile, const char* szClientIDFile, const char* szSSFile)
{
	//	Load configs ...
	if (!LoadConfigs(szCfgFile, szClientIDFile))
	{
		glb_ErrorOutput(ECERR_FAILEDTOCALL, "CECConfigs::Init", __LINE__);
		return false;
	}

	//	Load user settings ...
	if (!LoadSystemSettings(szSSFile))
	{
		glb_ErrorOutput(ECERR_FAILEDTOCALL, "CECConfigs::Init", __LINE__);
		return false;
	}

	// ensure the fullscreen resolution can be supported on this device
	if( m_ss.bFullScreen )
	{
#ifndef DX8
		IDirect3D9 *	pD3D;
		pD3D = Direct3DCreate9(D3D_SDK_VERSION);
#else
		IDirect3D8 *	pD3D;
		pD3D = Direct3DCreate8(D3D_SDK_VERSION);
#endif // ANGELICA_2_2
		if( pD3D == NULL )
		{
			glb_ErrorOutput(ECERR_FAILEDTOCALL, "CECConfigs::Init", __LINE__);
			return false;
		}

		A3DDevice::DISPLAYMODEARRAY displayModes;
#ifdef DX9
		//A3DDevice::EnumDisplayModes(pD3D, A3DFMT_X8R8G8B8, displayModes);

		// make sure fullscreen resolution == desktop resolution
		D3DDISPLAYMODE mode;
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	
		m_ss.iRndWidth = mode.Width;
		m_ss.iRndHeight = mode.Height;
		pD3D->Release();

#else
		A3DDevice::EnumDisplayModes(pD3D, A3DFMT_UNKNOWN, displayModes);

		int idMode;
		for(idMode=0; idMode<displayModes.GetSize(); idMode++)
		{
			if( (int)displayModes[idMode].Width == m_ss.iRndWidth && (int)displayModes[idMode].Height == m_ss.iRndHeight )
				break;
		}

		if( idMode == displayModes.GetSize() )
		{
			// now current device width and height are invalid on this device, engine will start chose best proper display mode
			a_LogOutput(1, "Fullscreen resolution[%dx%d] is not supported. Engine will chose one proper resolution",
				m_ss.iRndWidth, m_ss.iRndHeight);

			int nBestWidth = 0;
			int nBestHeight = 0;
			int nBestDiff = 0x7fffffff;
			for(idMode=0; idMode<displayModes.GetSize(); idMode++)
			{
				int nDiff = abs(int(displayModes[idMode].Width) - m_ss.iRndWidth) + 
					abs(int(displayModes[idMode].Height) - m_ss.iRndHeight);
				if( nDiff < nBestDiff  )
				{
					nBestWidth = displayModes[idMode].Width;
					nBestHeight = displayModes[idMode].Height;
					nBestDiff = nDiff;
				}
			}

			if( nBestWidth == 0 || nBestHeight == 0 )
			{
				a_LogOutput(1, "Can't find proper display mode for this device.");
				return false;
			}
			else
			{
				a_LogOutput(1, "Chose displaymode [%dx%d]", nBestWidth, nBestHeight);
				m_ss.iRndWidth = nBestWidth;
				m_ss.iRndHeight = nBestHeight;
			}
		}
		else
		{
		}
#endif // ANGELICA_2_2

	}

	return true;
}

//	Restore user settings to default value
void CECConfigs::DefaultUserSettings(EC_SYSTEM_SETTING* pss,
		EC_VIDEO_SETTING* pvs, EC_GAME_SETTING* pgs,
		EC_BLACKLIST_SETTING* pbs, EC_COMPUTER_AIDED_SETTING *pcas, EC_UI_SETTING *puis)
{
	if (pss) pss->Reset();
	if (pvs) pvs->Reset();
	if (pgs) pgs->Reset();
	if (pbs) pbs->Reset();
	if (pcas) pcas->Reset();
	if (puis) puis->Reset();
}

//	Load configs from file
bool CECConfigs::LoadConfigs(const char* szFile, const char* szClientIDFile)
{
	AWIniFile IniFile;

	if (!IniFile.Open(szFile))
	{
		a_LogOutput(1, "CECConfigs::LoadConfigs, Failed to open file %s", szFile);
		return false;
	}

	//	PathFile section
	ACString strSect = _AL("PathFile");	

	m_strThemeFiles.clear();
	ACString strValue = IniFile.GetValueAsString(strSect, _AL("theme"));
	if (!strValue.IsEmpty())
	{
		CSplit splitter(strValue);
		CSplit::VectorAWString strArrays = splitter.Split(_AL(","));
		for (size_t i(0); i < strArrays.size(); ++ i)
		{
			AWString strTemp = strArrays[i];
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if (!strTemp.IsEmpty())
				m_strThemeFiles.push_back(AC2AS(strTemp));
		}
	}		

	//	Settings section
	strSect = _AL("Settings");

	m_iCodePage		= IniFile.GetValueAsInt(strSect, _AL("code_page"), m_iCodePage);
	m_bConsole		= IniFile.GetValueAsInt(strSect, _AL("console"), m_bConsole ? 1 : 0) ? true : false;
	m_iRTDebug		= IniFile.GetValueAsInt(strSect, _AL("runtime_debug"), m_iRTDebug);
	m_iMaxNameLen	= IniFile.GetValueAsInt(strSect, _AL("max_name_len"), m_iMaxNameLen);
	m_iMultiClient	= IniFile.GetValueAsInt(strSect, _AL("multi_client"), m_iMultiClient);
	m_strWindowsTitle = IniFile.GetValueAsString(strSect, _AL("windows_title"), m_strWindowsTitle);
	m_strMiniDLTitle= IniFile.GetValueAsString(strSect, _AL("mini_dl_title"), m_strMiniDLTitle);
	m_iClientID	= IniFile.GetValueAsInt(strSect, _AL("client_id"), m_iClientID);
	m_bRegisterUIScriptFunc = IniFile.GetValueAsInt(strSect, _AL("register_script"), m_bRegisterUIScriptFunc) != 0;
	m_bEnableGT = false;
	if (CECCommandLine::GetEnableGT(m_bEnableGT))
		a_LogOutput(1, "CECConfigs::LoadConfigs, gt %s by commandline", m_bEnableGT ? "enabled" : "disabled");
	m_bEnableArc = false;
	if (CECCommandLine::GetEnableArc(m_bEnableArc))
		a_LogOutput(1, "CECConfigs::LoadConfigs, coreclient %s by commandline", m_bEnableArc ? "enabled" : "disabled");
	m_bEnableArcAsia = false;
	if (CECCommandLine::GetEnableArcAsia(m_bEnableArcAsia))
		a_LogOutput(1, "CECConfigs::LoadConfigs, arcasia %s by commandline", m_bEnableArcAsia ? "enabled" : "disabled");
	m_bMiniClient = IniFile.GetValueAsInt(strSect, _AL("mini_client"), m_bMiniClient) != 0;
	if (CECCommandLine::GetEnableMiniClient(m_bMiniClient))
		a_LogOutput(1, "CECConfigs::LoadConfigs, miniclient %s by commandline", m_bMiniClient ? "enabled" : "disabled");
	int iDebugLevel(0);
	if (CECCommandLine::GetRtDebugLevel(iDebugLevel)){
		SetRTDebugLevel(iDebugLevel);
	}
	m_bSendLogicCheckInfo = IniFile.GetValueAsInt(strSect, _AL("logiccheck_info"), m_bSendLogicCheckInfo) != 0;
	if (CECCommandLine::GetEnableLogicCheckInfo(m_bSendLogicCheckInfo))
		a_LogOutput(1, "CECConfigs::LoadConfigs, logiccheck_info %s by commandline", m_bSendLogicCheckInfo ? "enabled" : "disabled");
	
	//	Default environment value
	strSect = _AL("DefEnv");

	m_fDefFogStart		= IniFile.GetValueAsFloat(strSect, _AL("fog_start"), 250.0f);
	m_fDefFogEnd		= IniFile.GetValueAsFloat(strSect, _AL("fog_end"), 600.0f);
	m_fDefFogDensity	= IniFile.GetValueAsFloat(strSect, _AL("fog_density"), 1.0f);
	m_strDefSkyFile0	= AC2AS(IniFile.GetValueAsString(strSect, _AL("sky_image01"), _AL("5\\Roof.bmp")));
	m_strDefSkyFile1	= AC2AS(IniFile.GetValueAsString(strSect, _AL("sky_image02"), _AL("5\\01.bmp")));
	m_strDefSkyFile2	= AC2AS(IniFile.GetValueAsString(strSect, _AL("sky_image03"), _AL("5\\02.bmp")));

	int col[3];
	IniFile.GetValueAsIntArray(strSect, _AL("fog_color"), 3, col);
	m_dwDefFogColor = A3DCOLORRGB(col[0], col[1], col[2]);

	IniFile.GetValueAsIntArray(strSect, _AL("ambient"), 3, col);
	m_dwDefAmbient = A3DCOLORRGB(col[0], col[1], col[2]);

	IniFile.GetValueAsIntArray(strSect, _AL("light_diffuse"), 3, col);
	m_dwDefLightDiff = A3DCOLORRGB(col[0], col[1], col[2]);

	IniFile.GetValueAsIntArray(strSect, _AL("light_specular"), 3, col);
	m_dwDefLightSpec = A3DCOLORRGB(col[0], col[1], col[2]);

	IniFile.GetValueAsFloatArray(strSect, _AL("light_dir"), 3, m_vDefLightDir.m);
	m_vDefLightDir.Normalize();

	strSect = _AL("ServerConfig");
	m_playerMaxLevel = IniFile.GetValueAsInt(strSect, _AL("player_max_level"), 105);
	IniFile.Close();
		
	if (IniFile.Open(szClientIDFile)){
		//	如果存在 client_id 配置文件，则以此文件中的内容为准，没有则以之前的配置，以兼容性以前的处理
		m_iClientID	= IniFile.GetValueAsInt(_AL("Default"), _AL("client_id"), m_iClientID);
		IniFile.Close();
	}
	if (m_iClientID != -1){
		a_LogOutput(1, "client_id = %d", m_iClientID);
	}

	m_bDebugFog			= false;
	m_fDebugFogStart	= m_fDefFogStart;
	m_fDebugFogEnd		= m_fDefFogEnd;
	m_fDebugFogDensity	= m_fDebugFogDensity;
	m_dwDebugFogColor	= m_dwDefFogColor;

	return true;
}

//	Load system settings from local disk file
bool CECConfigs::LoadSystemSettings(const char* szFile)
{
	AIniFile IniFile;

	if (!IniFile.Open(szFile))
	{
		a_LogOutput(1, "CECConfigs::LoadSystemSettings, Failed to open file %s", szFile);
		a_LogOutput(1, "we use default settings to let users go in...");
		return true;
	}

	//	Video section
	AString strSect = "Video";

	m_ss.nLevel			= IniFile.GetValueAsInt(strSect, "Level", 0);
	m_ss.nSight			= IniFile.GetValueAsInt(strSect, "Sight", 0);
	m_ss.nWaterEffect	= IniFile.GetValueAsInt(strSect, "WaterEffect", 0);
	m_ss.bSimpleTerrain = IniFile.GetValueAsInt(strSect, "SimpleTerrain", 0) ? true: false;
	m_ss.nTreeDetail	= IniFile.GetValueAsInt(strSect, "TreeDetail", 0);
	m_ss.nGrassDetail	= IniFile.GetValueAsInt(strSect, "GrassDetail", 0);
	m_ss.nCloudDetail	= IniFile.GetValueAsInt(strSect, "CloudDetail", 0);
	m_ss.bShadow		= IniFile.GetValueAsInt(strSect, "Shadow", 0) ? true : false;
	m_ss.bMipMapBias	= IniFile.GetValueAsInt(strSect, "MipMapBias", 0) ? true : false;
	m_ss.bFullGlow		= IniFile.GetValueAsInt(strSect, "FullGlow", 0) ? true : false;
	m_ss.bSpaceWarp		= IniFile.GetValueAsInt(strSect, "SpaceWarp", 0) ? true : false;
	m_ss.bSunFlare		= IniFile.GetValueAsInt(strSect, "SunFlare", 0) ? true : false;
	m_ss.bAdvancedWater	= IniFile.GetValueAsInt(strSect, "AdvancedWaterUD", 0) ? true : false;

	m_ss.iRndWidth		= IniFile.GetValueAsInt(strSect, "RenderWid", 800);
	m_ss.iRndHeight		= IniFile.GetValueAsInt(strSect, "RenderHei", 600);
	m_ss.iTexDetail		= IniFile.GetValueAsInt(strSect, "TexDetail", 2);
	m_ss.iSoundQuality  = IniFile.GetValueAsInt(strSect, "SoundQuality", 2);
	m_ss.bFullScreen	= IniFile.GetValueAsInt(strSect, "FullScreen", 1) ? true : false;
	m_ss.bWideScreen	= IniFile.GetValueAsInt(strSect, "WideScreen", 0) ? true : false;
	m_ss.bScaleUI		= IniFile.GetValueAsInt(strSect, "ScaleUI", 0) ? true : false;
	m_ss.bVSync			= IniFile.GetValueAsInt(strSect, "VerticalSync", 1) ? true : false;
	m_ss.iGamma			= IniFile.GetValueAsInt(strSect, "Gamma", 41);

	//	Audio section
	strSect = "Audio";

	m_ss.nSoundVol = IniFile.GetValueAsInt(strSect, "SoundVol", 50);
	m_ss.nMusicVol = IniFile.GetValueAsInt(strSect, "MusicVol", 50);
	
	//	UI section
	m_ss.iTheme	= DEFAULT_UI_THEME_ID;

	//  Optimize section
	strSect = "Optimize";
	m_ops.gfx.bExcludeHost = IniFile.GetValueAsBoolean(strSect, "ExcludeHost", false);
	m_ops.gfx.bHidePlayerCast = IniFile.GetValueAsBoolean(strSect, "HidePlayerCast", false);
	m_ops.gfx.bHideNPCCast = IniFile.GetValueAsBoolean(strSect, "HideNPCCast", false);
	m_ops.gfx.bHidePlayerAttack = IniFile.GetValueAsBoolean(strSect, "HidePlayerAttack", false);
	m_ops.gfx.bHideNPCAttack = IniFile.GetValueAsBoolean(strSect, "HideNPCAttack", false);
	m_ops.gfx.bHidePlayerFly = IniFile.GetValueAsBoolean(strSect, "HidePlayerFly", false);
	m_ops.gfx.bHideNPCFly = IniFile.GetValueAsBoolean(strSect, "HideNPCFly", false);
	m_ops.gfx.bHidePlayerHit = IniFile.GetValueAsBoolean(strSect, "HidePlayerHit", false);
	m_ops.gfx.bHideNPCHit = IniFile.GetValueAsBoolean(strSect, "HideNPCHit", false);
	m_ops.gfx.bHidePlayerState = IniFile.GetValueAsBoolean(strSect, "HidePlayerState", false);
	m_ops.gfx.bHideNPCState = IniFile.GetValueAsBoolean(strSect, "HideNPCState", false);
	m_ops.gfx.bHideWeaponStone = IniFile.GetValueAsBoolean(strSect, "HideWeaponStone", false);
	m_ops.gfx.bHideArmorStone = IniFile.GetValueAsBoolean(strSect, "HideArmorStone", false);
	m_ops.gfx.bHideSuite = IniFile.GetValueAsBoolean(strSect, "HideSuite", false);
	m_ops.bAutoSimplify = IniFile.GetValueAsBoolean(strSect, "AutoModelSimplify", false);
	m_ops.iSimplifyMode = IniFile.GetValueAsInt(strSect, "SimplifyMode", CECMemSimplify::MEMUSAGE_NORMAL);
	
	// Read new memory optimization settings from INI (will be overwritten by binary config if it exists)
	m_ops.nConfigMode = IniFile.GetValueAsInt(strSect, "ConfigMode", 0);
	m_ops.nModelLimit = (BYTE)IniFile.GetValueAsInt(strSect, "ModelLimit", 50);
	m_ops.nMateLimit = (BYTE)IniFile.GetValueAsInt(strSect, "MateLimit", 100);
	m_ops.nEnemyLimit = (BYTE)IniFile.GetValueAsInt(strSect, "EnemyLimit", 100);
	m_ops.nSpEffect = (BYTE)IniFile.GetValueAsInt(strSect, "SpEffect", 0);
	m_ops.nEffectsDistance = (BYTE)IniFile.GetValueAsInt(strSect, "EffectsDistance", 100);
	m_ops.nEffectsType = (BYTE)IniFile.GetValueAsInt(strSect, "EffectsType", 0);
	m_ops.nNameDistance = (BYTE)IniFile.GetValueAsInt(strSect, "NameDistance", 100);
	m_ops.bPeopleLimit = IniFile.GetValueAsBoolean(strSect, "PeopleLimit", false);
	m_ops.bModelLimit = IniFile.GetValueAsBoolean(strSect, "ModelLimitEnabled", false);
	m_ops.bPeopleLimitByNear = IniFile.GetValueAsBoolean(strSect, "PeopleLimitByNear", false);
	m_ops.bSpEffect = IniFile.GetValueAsBoolean(strSect, "SpEffectEnabled", false);
	m_ops.bEffectsDistance = IniFile.GetValueAsBoolean(strSect, "EffectsDistanceEnabled", false);
	m_ops.bEffectsShow = IniFile.GetValueAsBoolean(strSect, "EffectsShow", false);
	m_ops.bExcludeHost = IniFile.GetValueAsBoolean(strSect, "ExcludeHostOptimize", m_ops.gfx.bExcludeHost); // Use gfx value as default
	m_ops.bTargetSimpleModel = IniFile.GetValueAsBoolean(strSect, "TargetSimpleModel", false);
	
	// Sync settings to m_ops.gfx after loading from INI
	m_ops.gfx.bExcludeHost = m_ops.bExcludeHost;
	m_ops.gfx.nSpEffect = m_ops.nSpEffect;
	m_ops.gfx.bSpEffect = m_ops.bSpEffect;
	m_ops.gfx.nEffectsDistance = m_ops.nEffectsDistance;
	m_ops.gfx.bEffectsDistance = m_ops.bEffectsDistance;
	m_ops.gfx.nEffectsType = m_ops.nEffectsType;
	m_ops.gfx.bEffectsShow = m_ops.bEffectsShow;

	IniFile.Close();
	
	// Note: ApplyOptimizeSetting() will be called after LoadUserConfigData() 
	// to ensure binary config (if exists) takes precedence and settings are applied once

	ConvertSightSetting(m_ss.nSight);

	m_fForestDetail = m_ss.nTreeDetail * 0.2f;
	m_fGrassLandDetail = m_ss.nGrassDetail * 0.25f;

	return true;
}

float CECConfigs::GetSceneLoadRadius()
{ 
	if (g_pGame->GetGameRun()->GetGameState() == CECGameRun::GS_GAME)
		return m_fSceLoadRadius; 
	else
		return 500.0f;
}

void CECConfigs::SetPlayerVisRadius(float fRadius)
{ 
	m_fCurPVRadius = fRadius; 
	a_Clamp(m_fCurPVRadius, m_fMinPVRadius, m_fSevActiveRad);
}

void CECConfigs::ToggleNameRenderRadius()
{
	// Toggle between default (60/20) and extended (80/30) radius
	if (m_fPlayerNameRenderRadius == 60.0f)
	{
		m_fPlayerNameRenderRadius = 80.0f;
		m_fNPCNameRenderRadius = 30.0f;
	}
	else
	{
		m_fPlayerNameRenderRadius = 60.0f;
		m_fNPCNameRenderRadius = 20.0f;
	}
}

void CECConfigs::ToggleModelRenderRadius()
{
	// Toggle between default (60) and extended (200) radius
	if (m_fModelRenderRadius == 60.0f)
	{
		m_fModelRenderRadius = 200.0f;
	}
	else
	{
		m_fModelRenderRadius = 60.0f;
	}
}

//	Save system settings to local disk
bool CECConfigs::SaveSystemSettings()
{
	AIniFile IniFile;

	//	Info section
	AString strSect = "Info";

	//	now write the video card information here
	char szInfo[1024];
#ifndef DX8
	D3DADAPTER_IDENTIFIER9 idAdapter;
	g_pGame->GetA3DEngine()->GetD3D()->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &idAdapter);
#else
	D3DADAPTER_IDENTIFIER8 idAdapter;
	g_pGame->GetA3DEngine()->GetD3D()->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_NO_WHQL_LEVEL, &idAdapter);
#endif // ANGELICA_2_2

	IniFile.WriteStringValue(strSect, "card", idAdapter.Description);
	IniFile.WriteStringValue(strSect, "driver", idAdapter.Driver);

	WORD Product = HIWORD(idAdapter.DriverVersion.HighPart);
	WORD Version = LOWORD(idAdapter.DriverVersion.HighPart);
	WORD SubVersion = HIWORD(idAdapter.DriverVersion.LowPart);
	WORD Build = LOWORD(idAdapter.DriverVersion.LowPart);
	sprintf(szInfo, "%d.%d.%04d.%04d", Product, Version, SubVersion, Build);
	IniFile.WriteStringValue(strSect, "version", szInfo);

	sprintf(szInfo, "%08x", idAdapter.VendorId);
	IniFile.WriteStringValue(strSect, "vendor", szInfo);
	sprintf(szInfo, "%08x", idAdapter.DeviceId);
	IniFile.WriteStringValue(strSect, "deviceid", szInfo);
	sprintf(szInfo, "%08x", idAdapter.SubSysId);
	IniFile.WriteStringValue(strSect, "subsysid", szInfo);
	sprintf(szInfo, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", idAdapter.DeviceIdentifier.Data1, idAdapter.DeviceIdentifier.Data2, idAdapter.DeviceIdentifier.Data3, 
		idAdapter.DeviceIdentifier.Data4[0], idAdapter.DeviceIdentifier.Data4[1], idAdapter.DeviceIdentifier.Data4[2], idAdapter.DeviceIdentifier.Data4[3],
		idAdapter.DeviceIdentifier.Data4[4], idAdapter.DeviceIdentifier.Data4[5], idAdapter.DeviceIdentifier.Data4[6], idAdapter.DeviceIdentifier.Data4[7]);
	IniFile.WriteStringValue(strSect, "guid", szInfo);
	
	//	Video section
	strSect = "Video";

	IniFile.WriteIntValue(strSect, "Level", m_ss.nLevel);
	IniFile.WriteIntValue(strSect, "Sight", m_ss.nSight);
	IniFile.WriteIntValue(strSect, "WaterEffect", m_ss.nWaterEffect);
	IniFile.WriteIntValue(strSect, "SimpleTerrain", m_ss.bSimpleTerrain);
	IniFile.WriteIntValue(strSect, "TreeDetail", m_ss.nTreeDetail);
	IniFile.WriteIntValue(strSect, "GrassDetail", m_ss.nGrassDetail);
	IniFile.WriteIntValue(strSect, "CloudDetail", m_ss.nCloudDetail);
	IniFile.WriteIntValue(strSect, "Shadow", m_ss.bShadow);
	IniFile.WriteIntValue(strSect, "MipMapBias", m_ss.bMipMapBias);
	IniFile.WriteIntValue(strSect, "FullGlow", m_ss.bFullGlow);
	IniFile.WriteIntValue(strSect, "SpaceWarp", m_ss.bSpaceWarp);
	IniFile.WriteIntValue(strSect, "SunFlare", m_ss.bSunFlare);
	IniFile.WriteIntValue(strSect, "AdvancedWaterUD", m_ss.bAdvancedWater);

	IniFile.WriteIntValue(strSect, "RenderWid", m_ss.iRndWidth);
	IniFile.WriteIntValue(strSect, "RenderHei", m_ss.iRndHeight);
	IniFile.WriteIntValue(strSect, "TexDetail", m_ss.iTexDetail);
	IniFile.WriteIntValue(strSect, "SoundQuality", m_ss.iSoundQuality);
	IniFile.WriteIntValue(strSect, "FullScreen", m_ss.bFullScreen);
	IniFile.WriteIntValue(strSect, "WideScreen", m_ss.bWideScreen);
	IniFile.WriteIntValue(strSect, "ScaleUI",	m_ss.bScaleUI);
	IniFile.WriteIntValue(strSect, "VerticalSync", m_ss.bVSync);
	IniFile.WriteIntValue(strSect, "Gamma", m_ss.iGamma);

	//	Audio section
	strSect = "Audio";

	IniFile.WriteIntValue(strSect, "SoundVol", m_ss.nSoundVol);
	IniFile.WriteIntValue(strSect, "MusicVol", m_ss.nMusicVol);
	
	//	UI section
	strSect = "UI";
	IniFile.WriteIntValue(strSect, "Theme", m_ss.iTheme);

	//  Optimize section
	const CECOptimize::GFX& gfx = CECOptimize::Instance().GetGFX();
	CECMemSimplify* pMemSimplify = g_pGame->GetGameRun()->GetMemSimplify();

	strSect = "Optimize";
	IniFile.WriteIntValue(strSect, "ExcludeHost", m_ops.gfx.bExcludeHost);
	IniFile.WriteIntValue(strSect, "HidePlayerCast", m_ops.gfx.bHidePlayerCast);
	IniFile.WriteIntValue(strSect, "HideNPCCast", m_ops.gfx.bHideNPCCast);
	IniFile.WriteIntValue(strSect, "HidePlayerAttack", m_ops.gfx.bHidePlayerAttack);
	IniFile.WriteIntValue(strSect, "HideNPCAttack", m_ops.gfx.bHideNPCAttack);
	IniFile.WriteIntValue(strSect, "HidePlayerFly", m_ops.gfx.bHidePlayerFly);
	IniFile.WriteIntValue(strSect, "HideNPCFly", m_ops.gfx.bHideNPCFly);
	IniFile.WriteIntValue(strSect, "HidePlayerHit", m_ops.gfx.bHideNPCHit);
	IniFile.WriteIntValue(strSect, "HideNPCHit", m_ops.gfx.bHideNPCHit);
	IniFile.WriteIntValue(strSect, "HidePlayerState", m_ops.gfx.bHidePlayerState);
	IniFile.WriteIntValue(strSect, "HideNPCState", m_ops.gfx.bHideNPCState);
	IniFile.WriteIntValue(strSect, "HideWeaponStone", m_ops.gfx.bHideWeaponStone);
	IniFile.WriteIntValue(strSect, "HideArmorStone", m_ops.gfx.bHideArmorStone);
	IniFile.WriteIntValue(strSect, "HideSuite", m_ops.gfx.bHideSuite);
	IniFile.WriteIntValue(strSect, "AutoModelSimplify", m_ops.bAutoSimplify);
	IniFile.WriteIntValue(strSect, "SimplifyMode", m_ops.iSimplifyMode);
	
	// Write new memory optimization settings
	IniFile.WriteIntValue(strSect, "ConfigMode", m_ops.nConfigMode);
	IniFile.WriteIntValue(strSect, "ModelLimit", m_ops.nModelLimit);
	IniFile.WriteIntValue(strSect, "MateLimit", m_ops.nMateLimit);
	IniFile.WriteIntValue(strSect, "EnemyLimit", m_ops.nEnemyLimit);
	IniFile.WriteIntValue(strSect, "SpEffect", m_ops.nSpEffect);
	IniFile.WriteIntValue(strSect, "EffectsDistance", m_ops.nEffectsDistance);
	IniFile.WriteIntValue(strSect, "EffectsType", m_ops.nEffectsType);
	IniFile.WriteIntValue(strSect, "NameDistance", m_ops.nNameDistance);
	IniFile.WriteIntValue(strSect, "PeopleLimit", m_ops.bPeopleLimit);
	IniFile.WriteIntValue(strSect, "ModelLimitEnabled", m_ops.bModelLimit);
	IniFile.WriteIntValue(strSect, "PeopleLimitByNear", m_ops.bPeopleLimitByNear);
	IniFile.WriteIntValue(strSect, "SpEffectEnabled", m_ops.bSpEffect);
	IniFile.WriteIntValue(strSect, "EffectsDistanceEnabled", m_ops.bEffectsDistance);
	IniFile.WriteIntValue(strSect, "EffectsShow", m_ops.bEffectsShow);
	IniFile.WriteIntValue(strSect, "ExcludeHostOptimize", m_ops.bExcludeHost);
	IniFile.WriteIntValue(strSect, "TargetSimpleModel", m_ops.bTargetSimpleModel);

	IniFile.Save("userdata\\SystemSettings.ini");
	IniFile.Close();

	return true;
}

//	Save user configs (except system settings) to specified buffer
bool CECConfigs::SaveUserConfigData(void* pDataBuf, int* piSize)
{
	int iTotalSize=0;
	BYTE* pData = (BYTE*)pDataBuf;

	//	Version
	iTotalSize += sizeof (DWORD);
	if (pDataBuf)
	{
		*((DWORD*)pData) = EC_CONFIG_VERSION;
		pData += sizeof (DWORD);
	}

	iTotalSize += sizeof (EC_VIDEO_SETTING);
	if (pDataBuf)
	{
		*((EC_VIDEO_SETTING*)pData) = m_vs;
		pData += sizeof (EC_VIDEO_SETTING);
	}

	iTotalSize += sizeof (EC_GAME_SETTING);
	if (pDataBuf)
	{
		*((EC_GAME_SETTING*)pData) = m_gs;
		pData += sizeof (EC_GAME_SETTING);
	}

	iTotalSize += sizeof (EC_BLACKLIST_SETTING);
	if (pDataBuf)
	{
		*((EC_BLACKLIST_SETTING*)pData) = m_bs;
		pData += sizeof (EC_BLACKLIST_SETTING);
	}
	
	iTotalSize += sizeof (EC_COMPUTER_AIDED_SETTING);
	if (pDataBuf)
	{
		*((EC_COMPUTER_AIDED_SETTING*)pData) = m_cas;
		pData += sizeof (EC_COMPUTER_AIDED_SETTING);
	}

	iTotalSize += sizeof (EC_UI_SETTING);
	if (pDataBuf)
	{
		*((EC_UI_SETTING*)pData) = m_uis;
		pData += sizeof (EC_UI_SETTING);
	}

	iTotalSize += sizeof (EC_OPTIMIZE_SETTING);
	if (pDataBuf)
	{
		// Sync settings from m_ops to m_ops.gfx before saving to ensure consistency
		m_ops.gfx.bExcludeHost = m_ops.bExcludeHost;
		m_ops.gfx.nSpEffect = m_ops.nSpEffect;
		m_ops.gfx.bSpEffect = m_ops.bSpEffect;
		m_ops.gfx.nEffectsDistance = m_ops.nEffectsDistance;
		m_ops.gfx.bEffectsDistance = m_ops.bEffectsDistance;
		m_ops.gfx.nEffectsType = m_ops.nEffectsType;
		m_ops.gfx.bEffectsShow = m_ops.bEffectsShow;
		
		*((EC_OPTIMIZE_SETTING*)pData) = m_ops;
		pData += sizeof (EC_OPTIMIZE_SETTING);
	}

	if (piSize)
		*piSize = iTotalSize;

	return true;
}

//	Set default user config data
void CECConfigs::DefaultUserConfigData()
{
	DefaultUserSettings(NULL, &m_vs, &m_gs, &m_bs, &m_cas, &m_uis);

	//	This fix a bug in some version
	m_gs.fCamTurnSpeed = 10.0f;
	m_gs.fCamZoomSpeed = 1.0f;
}

//	Load shortcut configs (except system settings) from specified buffer
bool CECConfigs::LoadUserConfigData(const void* pDataBuf, int iDataSize)
{
	DWORD dwVer = 0; // Declare outside try block for use after catch
	try
	{
		CECDataReader dr((void*)pDataBuf, iDataSize);

		//	Version number
		dwVer = dr.Read_DWORD();

		//	Note: Return directly means reset all settings to default value !!!
		//		A better method should be used when version updated.
		//
		//  We start to write compatibility code from version 15.
		if (dwVer < 15)
		{
			DefaultUserConfigData();
			goto End;
		}
		else if (dwVer > EC_CONFIG_VERSION)
		{
			ASSERT(dwVer > EC_CONFIG_VERSION);
			throw CECException(CECException::TYPE_DATAERR);
		}

		m_vs.Read(dr, dwVer);
		m_gs.Read(dr, dwVer);
		m_bs.Read(dr, dwVer);
		m_cas.Read(dr, dwVer);
		if (dwVer >= 39)
			m_uis.Read(dr, dwVer);
		if (dwVer >= 40)
		{
			// Save current optimize settings from INI before loading from binary
			// This allows us to preserve INI settings if they are custom (ConfigMode = 2)
			EC_OPTIMIZE_SETTING savedOps = m_ops;
			
			m_ops.Read(dr, dwVer);
			
			// If INI has custom settings (ConfigMode = 2), they take priority over binary config
			// This ensures user's manual customizations are preserved
			if (savedOps.nConfigMode == 2)
			{
				m_ops = savedOps; // Use INI settings (custom config)
			}
			// Otherwise use binary config (preset Normal or Quick)
			
			// Sync settings from m_ops to m_ops.gfx after loading
			// This ensures gfx settings are up-to-date with optimize settings
			m_ops.gfx.bExcludeHost = m_ops.bExcludeHost;
			m_ops.gfx.nSpEffect = m_ops.nSpEffect;
			m_ops.gfx.bSpEffect = m_ops.bSpEffect;
			m_ops.gfx.nEffectsDistance = m_ops.nEffectsDistance;
			m_ops.gfx.bEffectsDistance = m_ops.bEffectsDistance;
			m_ops.gfx.nEffectsType = m_ops.nEffectsType;
			m_ops.gfx.bEffectsShow = m_ops.bEffectsShow;
		}
		else
		{
			// If binary config version < 40, optimize settings were not saved in binary
			// Keep the settings loaded from INI file (already loaded in LoadSystemSettings)
		}
	}
	catch (CECException& e)
	{
		ASSERT(0);
		a_LogOutput(1, "CECConfigs::LoadUserConfigData, data read error (%d)", e.GetType());
		DefaultUserConfigData();
	}

End:

	if (CECCrossServer::Instance().IsOnSpecialServer())
		memset(m_bs.idPlayer, 0, sizeof(m_bs.idPlayer));

	ApplyUserSetting();
	// Apply optimize settings after loading (since m_ops was loaded directly, we need to apply it)
	DWORD dwLoadedVer = dwVer; // Save version for later use
	if (dwLoadedVer >= 40)
		ApplyOptimizeSetting();
	
	// Apply UI settings after loading (scale dialogs)
	if (dwLoadedVer >= 39)
		ApplyUISettings();

	return true;
}

void CECConfigs::ApplyUserSetting()
{
	//
	// *NOTICE*: update this function after new value type added
	//
	// Try to verify the config data to avoid invalid values
	Verify();

	//	Build player and NPC text flags
	BuildTextFlags();
	
	m_fCurPVRadius = CalcPlayerVisRadius(m_vs.nDistance);
	g_pGame->GetA3DGFXExMan()->SetPriority(m_vs.nEffect);
	
	//  Send force attack to server
	BYTE forceAttack = glb_BuildPVPMask(false);
	BYTE refuseBless = glb_BuildRefuseBLSMask();
	g_pGame->GetGameSession()->c2s_CmdNotifyForceAttack(forceAttack, refuseBless);
}

void CECConfigs::SetSceneLoadRadius(float fRadius)
{
	m_fSceLoadRadius = fRadius;
	a_ClampFloor(m_fSceLoadRadius, m_fSevActiveRad);
	g_pGame->GetGameRun()->GetWorld()->SetViewRadius(fRadius);
}

void CECConfigs::SetForestDetail(float fDetail)
{
	m_fForestDetail = fDetail;
	a_Clamp(m_fForestDetail, 0.0f, 1.0f);
	g_pGame->GetGameRun()->GetWorld()->GetForest()->SetLODLevel(m_fForestDetail);
}

void CECConfigs::SetGrassLandDetail(float fDetail)
{
	m_fGrassLandDetail = fDetail;
	a_Clamp(m_fGrassLandDetail, 0.0f, 1.0f);
	g_pGame->GetGameRun()->GetWorld()->GetGrassLand()->SetLODLevel(m_fGrassLandDetail);
}

extern volatile bool g_bMultiThreadRenderMode;
extern void SwitchRenderThreadMode(bool bMultiThread);

void CECConfigs::SetSystemSettings(const EC_SYSTEM_SETTING& systemSetting, bool bCallFromWndProc)
{
	if (!g_pGame->GetGameRun() ||
		!g_pGame->GetGameRun()->GetWorld())
	{
		return;
	}

	EC_SYSTEM_SETTING ss = systemSetting;
	ApplyWorldSpecificSetting(ss);
	ss.iTheme = DEFAULT_UI_THEME_ID;

	CECWorld* pWorld = g_pGame->GetGameRun()->GetWorld();

	bool bDisplayModeChanged = 
		m_ss.iRndHeight != ss.iRndHeight || 
		m_ss.iRndWidth != ss.iRndWidth || 
		m_ss.bFullScreen != ss.bFullScreen || 
		m_ss.bWideScreen != ss.bWideScreen ||
		m_ss.bVSync != ss.bVSync;

	m_bAdjusting = true;

	//	Apply water effect
	A3DTerrainWater* pWater = pWorld->GetTerrainWater();
	if (pWater)
	{
		pWater->SetSimpleWaterFlag(!ss.nWaterEffect);
		pWater->SetExpensiveWaterFlag(ss.bAdvancedWater);
	}

	if( g_pGame->GetShadowRender() )
	{
		g_pGame->GetShadowRender()->SetCastShadowFlag(ss.bShadow);
		// we decide whether use triangle man according to shadow cast flag.
		if( g_pGame->GetShadowRender()->GetCastShadowFlag() )
			pWorld->GetOrnamentMan()->LoadTriangleMan(g_pGame->GetGameRun()->GetSafeHostPos(g_pGame->GetViewport()));
		else
			pWorld->GetOrnamentMan()->ReleaseTriangleMan(g_pGame->GetGameRun()->GetSafeHostPos(g_pGame->GetViewport()));
	}

	if( g_pGame->GetGameRun()->GetFullGlowRender() )
	{
		g_pGame->GetGameRun()->GetFullGlowRender()->SetGlowOn(ss.bFullGlow);
		g_pGame->GetGameRun()->GetFullGlowRender()->SetWarpOn(ss.bSpaceWarp);
		g_pGame->GetGameRun()->GetFullGlowRender()->SetFlareOn(ss.bSunFlare);
	}
	
	if (bDisplayModeChanged)
	{
		DWORD dwExStyle = 0;
		DWORD dwStyles = 0;
		int x, y, w, h;
		A3DFORMAT fmtTarget = A3DFMT_UNKNOWN;

		HWND hDevWnd = g_pGame->GetA3DDevice()->GetDeviceWnd();
		const A3DDevice::DISPLAY_MODE& desktopMode = g_pGame->GetA3DDevice()->GetDesktopDisplayMode();

		if( ss.bFullScreen )
		{
			dwStyles = WS_POPUP;
			dwExStyle = WS_EX_TOPMOST;

#ifdef DX9
			// make sure fullscreen resolution == desktop resolution
			D3DDISPLAYMODE mode;
			g_pGame->GetA3DEngine()->GetD3D()->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);

			ss.iRndWidth = mode.Width;
			ss.iRndHeight = mode.Height;
#endif
			x = 0;
			y = 0;
			w = ss.iRndWidth;
			h = ss.iRndHeight;
			if( g_pGame->GetA3DDevice()->GetTarget32() != A3DFMT_UNKNOWN )
				fmtTarget = g_pGame->GetA3DDevice()->GetTarget32();
			else
				fmtTarget = g_pGame->GetA3DDevice()->GetTarget16();
		}
		else
		{
			dwStyles |= WS_POPUP | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
			
			RECT rcWnd = {0, 0, ss.iRndWidth, ss.iRndHeight};
			AdjustWindowRectEx(&rcWnd, dwStyles, FALSE, dwExStyle);

			w = rcWnd.right - rcWnd.left;
			h = rcWnd.bottom - rcWnd.top;
			x = (desktopMode.nWidth - w) / 2;
			y = (desktopMode.nHeight - h) / 2;

			if( desktopMode.nBitsPerPixel == 32 && g_pGame->GetA3DDevice()->GetTarget32() != A3DFMT_UNKNOWN )
				fmtTarget = g_pGame->GetA3DDevice()->GetTarget32();
			else
				fmtTarget = g_pGame->GetA3DDevice()->GetTarget16();
		}

		SetWindowLong(hDevWnd, GWL_STYLE, dwStyles);
		SetWindowLong(hDevWnd, GWL_EXSTYLE, dwExStyle);

		if( !ss.bFullScreen )
		{
			if( m_ss.bFullScreen )
			{
				// switch from fullscreen to window mode, we need center the window; and redraw all windows
				SetWindowPos(hDevWnd, HWND_NOTOPMOST, x, y, w, h, SWP_SHOWWINDOW);
			}
			else
			{
				SetWindowPos(hDevWnd, HWND_NOTOPMOST, x, y, w, h, SWP_SHOWWINDOW | SWP_NOMOVE);

				if( !bCallFromWndProc )
					InvalidateRect(NULL, NULL, TRUE);
			}
		}
		else
		{
			SetWindowPos(hDevWnd, HWND_TOPMOST, x, y, w, h, SWP_SHOWWINDOW);
		}

		// we must suspend the loader thread before we can do reset of the device, otherwise we may fail when reset the devcie
		// because unmanaged object are still being added into the world.
		SuspendLoadThread();
		g_bMultiThreadRenderMode = false;
		g_pGame->GetA3DEngine()->SetDisplayMode(ss.iRndWidth, ss.iRndHeight, fmtTarget, !ss.bFullScreen, ss.bVSync, SDM_WIDTH | SDM_HEIGHT | SDM_FORMAT | SDM_WINDOW | SDM_VSYNC);
		StartLoaderThread();
		SwitchRenderThreadMode(!ss.bFullScreen);

		CECViewport* pViewport = g_pGame->GetViewport();
		pViewport->Move(0, 0, ss.iRndWidth, ss.iRndHeight, ss.bWideScreen);

		// now reload water resources;
		if (pWater)
		{
			pWater->ReloadWaterResources();
		}

		// now recreate full screen glow render
		CECFullGlowRender * pFullGlowRender = g_pGame->GetGameRun()->GetFullGlowRender();
		if( pFullGlowRender )
		{
			pFullGlowRender->ReloadResource();
		}
	}

	//	Apply view radius
	if (m_ss.nSight != ss.nSight && !pWorld->IsRandomMap())
	{
		//	Apply sight radius
		ConvertSightSetting(ss.nSight);
		SetSceneLoadRadius(m_fSceLoadRadius);

		A3DTerrain2* pTerrain = pWorld->GetTerrain();
		pTerrain->SetLODDist(m_fTrnLODDist1, m_fTrnLODDist2);

		CECSceneBlock::SetResLoadDists(ss.nSight + 1);
	}

	if( m_ss.nTreeDetail != ss.nTreeDetail )
	{
		//	Apply forest detail
		SetForestDetail(ss.nTreeDetail * 0.25f);
	}

	if( m_ss.nGrassDetail != ss.nGrassDetail )
	{
		//	Apply grass land detail
		SetGrassLandDetail(ss.nGrassDetail * 0.25f);
	}

	if( m_ss.nCloudDetail != ss.nCloudDetail )
	{
		if( g_pGame->GetGameRun()->GetWorld()->GetCloudManager() )
			g_pGame->GetGameRun()->GetWorld()->GetCloudManager()->SetCloudLevel(ss.nCloudDetail * 25 / 10);
	}

	if( m_ss.bMipMapBias != ss.bMipMapBias || bDisplayModeChanged) 
	{
		float v = ss.bMipMapBias ? -1.0f : 0.0f;
		for(int i=0; i<g_pGame->GetA3DDevice()->GetMaxSimultaneousTextures(); i++)
		{
#ifndef DX8
			g_pGame->GetA3DDevice()->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *(DWORD *)&v);
#else
			g_pGame->GetA3DDevice()->SetDeviceTextureStageState(i, D3DTSS_MIPMAPLODBIAS, *(DWORD *)&v);
#endif // ANGELICA_2_2
		}
	}

	if( m_ss.bVSync != ss.bVSync )
	{
		// after changing the present intervals for fullscreen mode, the gamma level will be restored to defaults (on ATI cards)
		// and the same gamme level will not be accepted by system. so here we first change it to a different values and then
		// set it back.
		g_pGame->GetA3DDevice()->SetGammaLevel(100);
	}
	
	g_pGame->GetA3DDevice()->SetGammaLevel((int)(30+ss.iGamma*1.7f));

	if( pWorld->GetTerrainOutline() )
	{
		if( ss.bSimpleTerrain )
		{
			// lowest terrain detail, use terrain outline replace
			pWorld->GetTerrainOutline()->SetReplaceTerrain(true);
		}
		else
		{
			pWorld->GetTerrainOutline()->SetReplaceTerrain(false);
		}
	}

	// now we can change the texture quality at runtime.
	if( m_ss.iTexDetail != ss.iTexDetail )
	{
		g_pA3DConfig->SetTextureQuality((A3DTEXTURE_QUALITY)ss.iTexDetail);
		g_pGame->GetA3DEngine()->GetA3DTextureMan()->ReloadAllTextures(true);
	}

	// now we can change the sound quality at runtime.
	if( m_ss.iSoundQuality != ss.iSoundQuality )
	{
		g_pGame->GetA3DEngine()->GetAMEngine()->GetAMConfig()->SetSoundQuality((AMSOUND_QUALITY) ss.iSoundQuality);
		g_pGame->GetA3DEngine()->GetAMEngine()->GetAMSoundEngine()->GetAMSoundBufferMan()->ReloadAll();
	}

	// 如果窗口大小不变，但界面缩放策略改变
	bool bChangeUIScale = false;
	if (m_ss.iRndHeight == ss.iRndHeight &&
		m_ss.iRndWidth == ss.iRndWidth &&
		m_ss.bScaleUI != ss.bScaleUI)
	{
		// 仅缩放策略改变，则需要重新设置缩放
		bChangeUIScale = true;
	}

	bool bChangeTheme = (m_ss.iTheme != ss.iTheme);

	m_ss = ss;

	if(bChangeTheme) // do not need to change scale
	{
		g_pGame->GetGameRun()->GetUIManager()->ResetTheme();
	}
	else if (bChangeUIScale)
	{
		CECGameUIMan *pGameUIMan = g_pGame->GetGameRun()->GetUIManager()->GetInGameUIMan();
		if (pGameUIMan)
			pGameUIMan->OnWindowScalePolicyChange();
	}

	m_bAdjusting = false;
}

void CECConfigs::SetVideoSettings(const EC_VIDEO_SETTING& vs)
{
	m_vs = vs;

	//	Build player and NPC text flags
	BuildTextFlags();

	m_fCurPVRadius = CalcPlayerVisRadius(m_vs.nDistance);

	g_pGame->GetA3DGFXExMan()->SetPriority(m_vs.nEffect);
}

void CECConfigs::SetGameSettings(const EC_GAME_SETTING& gs)
{
	m_gs = gs;
}

void CECConfigs::SetGameSettings_NoOptimizeFunction(const EC_GAME_SETTING& gs)
{
	bool bPetAutoSkill = m_gs.bPetAutoSkill;
	bool bLockQuickBar = m_gs.bLockQuickBar;
	bool bHideIceThunderBall = m_gs.bHideIceThunderBall;
	bool bHideElf = m_gs.bHideElf;
	bool bHideQuest = m_gs.bHideQuest;
	bool bNormalBuffDisplay = m_gs.bNormalBuffDisplay;
	bool bUnfreeze = m_gs.bUnfreeze;
	bool bOffFly = m_gs.bOffFly;
	bool bOffHair = m_gs.bOffHair;
	bool bNoDelay = m_gs.bNoDelay;
	bool bAutoComplete = m_gs.bAutoComplete;
	bool bOffUnselect = m_gs.bOffUnselect;
	bool bCDTimer = m_gs.bCDTimer;
	bool bOffWeapon = m_gs.bOffWeapon;
	bool bFastFly = m_gs.bFastFly;
	bool bOffBooth = m_gs.bOffBooth;
	m_gs = gs;
	m_gs.bPetAutoSkill = bPetAutoSkill;
	m_gs.bLockQuickBar = bLockQuickBar;
	m_gs.bHideIceThunderBall = bHideIceThunderBall;
	m_gs.bHideElf = bHideElf;
	m_gs.bHideQuest = bHideQuest;
	m_gs.bNormalBuffDisplay = bNormalBuffDisplay;
	m_gs.bUnfreeze = bUnfreeze;
	m_gs.bOffFly = bOffFly;
	m_gs.bOffHair = bOffHair;
	m_gs.bNoDelay = bNoDelay;
	m_gs.bAutoComplete = bAutoComplete;
	m_gs.bOffUnselect = bOffUnselect;
	m_gs.bCDTimer = bCDTimer;
	m_gs.bOffWeapon = bOffWeapon;
	m_gs.bFastFly = bFastFly;
	m_gs.bOffBooth = bOffBooth;
}

void CECConfigs::SetGameSettings_OptimizeFunction(const EC_GAME_SETTING& gs)
{
	m_gs.bPetAutoSkill = gs.bPetAutoSkill;
	m_gs.bLockQuickBar = gs.bLockQuickBar;
	m_gs.bHideIceThunderBall = gs.bHideIceThunderBall;
	m_gs.bUnfreeze = gs.bUnfreeze;
	m_gs.bHideElf = gs.bHideElf;
	m_gs.bHideQuest = gs.bHideQuest;
	m_gs.bNormalBuffDisplay = gs.bNormalBuffDisplay;
	m_gs.bOffFly = gs.bOffFly;
	m_gs.bOffHair = gs.bOffHair;
	m_gs.bNoDelay = gs.bNoDelay;
	m_gs.bAutoComplete = gs.bAutoComplete;
	m_gs.bOffUnselect = gs.bOffUnselect;
	m_gs.bCDTimer = gs.bCDTimer;
	m_gs.bOffWeapon = gs.bOffWeapon;
	m_gs.bFastFly = gs.bFastFly;
	m_gs.bOffBooth = gs.bOffBooth;
	
	// Apply bUnfreeze setting immediately
	g_bRenderNoFocus = m_gs.bUnfreeze;
}

void CECConfigs::SetBlackListSettings(const EC_BLACKLIST_SETTING& bs)
{
	m_bs = bs;
}

void CECConfigs::SetComputerAidedSetting(const EC_COMPUTER_AIDED_SETTING& cas)
{
	m_cas = cas;
}

void CECConfigs::SetUISettings(const EC_UI_SETTING& uis)
{
	m_uis = uis;
	ApplyUISettings();
}

void CECConfigs::ApplyUISettings()
{
	// Apply UI scales to dialogs after loading or changing settings
	// Only apply if game is running and UI manager is ready
	if (!g_pGame || !g_pGame->GetGameRun())
		return;
	
	// Don't apply settings if UI manager is not initialized yet (during loading)
	CECUIManager* pUIManager = g_pGame->GetGameRun()->GetUIManager();
	if (!pUIManager)
		return;
	
	CECGameUIMan* pUIMan = (CECGameUIMan*)pUIManager->GetInGameUIMan();
	if (!pUIMan)
		return;

	PAUIDIALOG pDlg;
	
	// Apply target scale to all target dialogs
	float fTargetScale = EC_UI_SETTING::LevelToScale(m_uis.nScaleTarget);
	const char* szTargetDialogs[] = {
		"Win_HpOtherB",
		"Win_HpOther",
		"Win_Hpmpxp"
	};
	for (int i = 0; i < sizeof(szTargetDialogs) / sizeof(szTargetDialogs[0]); i++)
	{
		pDlg = pUIMan->GetDialog(szTargetDialogs[i]);
		if (pDlg)
			pDlg->SetScale(fTargetScale);
	}
	
	pDlg = pUIMan->GetDialog("Win_Map");
	if (pDlg)
		pDlg->SetScale(EC_UI_SETTING::LevelToScale(m_uis.nScaleMap));
	
	// Apply quickbar scale to all quickbar dialogs
	float fQuickbarScale = EC_UI_SETTING::LevelToScale(m_uis.nScaleQuickbar);
	const char* szQuickbarDialogs[] = {
		"Win_Quickbar13Va",
		"Win_Quickbar13Vb_1", "Win_Quickbar13Vb_2", "Win_Quickbar13Vb_3", "Win_Quickbar13Vb_4", "Win_Quickbar13Vb_5",
		"Win_Quickbar12Ha", "Win_Quickbar12Hb_1", "Win_Quickbar12Hb_2", "Win_Quickbar12Hb_3",
		"Win_Quickbar12Va", "Win_Quickbar12Vb_1", "Win_Quickbar12Vb_2", "Win_Quickbar12Vb_3",
		"Win_Quickbar13Ha", "Win_Quickbar13Hb_1", "Win_Quickbar13Hb_2", "Win_Quickbar13Hb_3", "Win_Quickbar13Hb_4", "Win_Quickbar13Hb_5"
	};
	for (int i = 0; i < sizeof(szQuickbarDialogs) / sizeof(szQuickbarDialogs[0]); i++)
	{
		pDlg = pUIMan->GetDialog(szQuickbarDialogs[i]);
		if (pDlg)
			pDlg->SetScale(fQuickbarScale);
	}
	
	// Apply chat scale to all chat dialogs
	float fChatScale = EC_UI_SETTING::LevelToScale(m_uis.nScaleChat);
	pDlg = pUIMan->GetDialog("Win_Chat");
	if (pDlg)
		pDlg->SetScale(fChatScale);
	pDlg = pUIMan->GetDialog("Win_ChatSmall");
	if (pDlg)
		pDlg->SetScale(fChatScale);
	
	// Apply team scale to all team dialogs
	float fTeamScale = EC_UI_SETTING::LevelToScale(m_uis.nScaleTeam);
	const char* szTeamDialogs[] = {
		"Win_TeamMain",
		"Win_TeamMate1", "Win_TeamMate2", "Win_TeamMate3", "Win_TeamMate4", "Win_TeamMate5",
		"Win_TeamMate6", "Win_TeamMate7", "Win_TeamMate8", "Win_TeamMate9"
	};
	for (int i = 0; i < sizeof(szTeamDialogs) / sizeof(szTeamDialogs[0]); i++)
	{
		pDlg = pUIMan->GetDialog(szTeamDialogs[i]);
		if (pDlg)
			pDlg->SetScale(fTeamScale);
	}
	
	pDlg = pUIMan->GetDialog("Win_ELF");
	if (pDlg)
		pDlg->SetScale(EC_UI_SETTING::LevelToScale(m_uis.nScaleElf));
	
	// Apply amulet scale to all amulet dialogs
	float fAmuletScale = EC_UI_SETTING::LevelToScale(m_uis.nScaleAmulet);
	const char* szAmuletDialogs[] = {
		"Win_Amulet",
		"Win_AmuletMain",
		"Win_PrgsHP",
		"Win_PrgsMP"
	};
	for (int i = 0; i < sizeof(szAmuletDialogs) / sizeof(szAmuletDialogs[0]); i++)
	{
		pDlg = pUIMan->GetDialog(szAmuletDialogs[i]);
		if (pDlg)
			pDlg->SetScale(fAmuletScale);
	}
	
	pDlg = pUIMan->GetDialog("Win_Inventory");
	if (pDlg)
		pDlg->SetScale(EC_UI_SETTING::LevelToScale(m_uis.nScaleInventory));
	
	// Try common option/setting dialog names
	pDlg = pUIMan->GetDialog("Win_Option");
	if (!pDlg) pDlg = pUIMan->GetDialog("Win_SettingSystem");
	if (!pDlg) pDlg = pUIMan->GetDialog("Win_SettingGame");
	if (pDlg)
		pDlg->SetScale(EC_UI_SETTING::LevelToScale(m_uis.nScaleOption));
	
	// Apply FPS display setting is now synchronized in EC_GameSession::OnPrtcGetConfigRe
	// after player is fully initialized to avoid crashes during loading
	// (FPS flag will be set there after OnAllInitDataReady)
	
	// Update window title based on setting
	// Don't update window title here during initialization - it will be updated when player is ready
	// UpdateWindowTitle();
}

void CECConfigs::UpdateWindowTitle()
{
	// Extra safety checks to prevent freezing
	if (!g_pGame || !g_pGame->GetA3DDevice())
		return;
	
	HWND hDevWnd = g_pGame->GetA3DDevice()->GetDeviceWnd();
	if (!hDevWnd || !IsWindow(hDevWnd))
		return;
	
	if (m_uis.bSetCustomWindowTitle)
	{
		// Custom window title: show player name and profession class
		// Only update if game is running and host player is ready
		if (g_pGame->GetGameRun() && g_pGame->GetGameRun()->GetHostPlayer())
		{
			CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();
			if (pHost && pHost->HostIsReady())
			{
				ACString strPlayerName = pHost->GetName();
				if (!strPlayerName.IsEmpty() && strPlayerName[0] != 0)
				{
					// Safely get profession name - GetProfName might not be ready yet
					ACString strTitle = strPlayerName;
					
					// Extra safety: check if GetProfName is safe to call
					int iProf = pHost->GetProfession();
					if (iProf >= 0 && iProf < 100 && g_pGame->GetGameRun()) // Sanity check on profession ID
					{
						const wchar_t* szProfName = g_pGame->GetGameRun()->GetProfName(iProf);
						if (szProfName && szProfName[0])
						{
							strTitle.Format(_AL("%s - %s"), strPlayerName, ACString(szProfName));
						}
					}
					
					::SetWindowTextW(hDevWnd, strTitle);
					return;
				}
			}
		}
		// If player not ready yet, use default title (will be updated later)
	}
	
	// Default window title
	::SetWindowTextW(hDevWnd, m_strWindowsTitle);
}

void CECConfigs::SetModelOptimize(bool bAutoSimplify, int iSimplifyMode)
{
	m_ops.bAutoSimplify = bAutoSimplify;
	m_ops.iSimplifyMode = iSimplifyMode;
}

//	Convert sight radius setting
void CECConfigs::ConvertSightSetting(int iSight)
{
	//	Apply sight radius
	switch (iSight)
	{
	case 0:	
		
		m_fSceLoadRadius = m_fSevActiveRad;
		m_fTrnLODDist1 = 40.0f;
		m_fTrnLODDist2 = 80.0f;
		break;

	case 1:	
		
		m_fSceLoadRadius = 250.0f;
		m_fTrnLODDist1 = 120.0f;
		m_fTrnLODDist2 = 240.0f;
		break;

	case 2:
	case 3:
	case 4:
		
		m_fSceLoadRadius = 420.0f;
		m_fTrnLODDist1 = 180.0f;
		m_fTrnLODDist2 = 320.0f;
		break;

	default:		
		ASSERT(0);
		return;
	}
}

//	Build player and NPC text flags
void CECConfigs::BuildTextFlags()
{
	//	Player name flags
	m_dwPlayerText = 0;

	if (m_vs.bPlayerHeadText)
	{
		m_dwPlayerText |= m_vs.bPlayerName ? CECPlayer::RNF_NAME : 0;
		m_dwPlayerText |= m_vs.bPlayerTitle ? CECPlayer::RNF_TITLE : 0;
		m_dwPlayerText |= m_vs.bPlayerFaction ? CECPlayer::RNF_FACTION : 0;
		m_dwPlayerText |= m_vs.bPlayerTalk ? CECPlayer::RNF_WORDS : 0;
		m_dwPlayerText |= m_vs.bPlayerForce ? CECPlayer::RNF_FORCE : 0;
	}

	if (m_vs.bPlayerShop)
	{
		m_dwPlayerText |= m_vs.bPlayerShop ? CECPlayer::RNF_BUY : 0;
		m_dwPlayerText |= m_vs.bPlayerShop ? CECPlayer::RNF_SELL : 0;
	}

	//	NPC name flags
	m_dwNPCText	= 0;

	m_dwNPCText |= m_vs.bNPCName ? CECNPC::RNF_NPCNAME : 0;
	m_dwNPCText |= m_vs.bMonsterName ? CECNPC::RNF_MONSTERNAME : 0;
	m_dwNPCText |= CECNPC::RNF_WORDS;
}

void CECConfigs::Verify()
{
	// Try to fix the invalid data
#define VERIFY_INVALID(data, value)	{	\
	ASSERT(FALSE);	\
	a_LogOutput(1, "CECConfigs::Verify, found invalid " #data " == %d", (int)data);	\
	data = value;}	\
	//

	// Verify game setting
	m_gs.szAutoReply[EC_AUTOREPLY_LEN] = '\0';
	if(m_gs.nFontSize > 4) VERIFY_INVALID(m_gs.nFontSize, 4);
	
	//	This fix a bug in some version
	if(m_gs.fCamTurnSpeed != 10.f) VERIFY_INVALID(m_gs.fCamTurnSpeed, 10.f);
	if(m_gs.fCamZoomSpeed != 1.f) VERIFY_INVALID(m_gs.fCamZoomSpeed, 1.f);
	
	// Verify general video setting
	if(m_vs.nDistance > 100) VERIFY_INVALID(m_vs.nDistance, 100);
	if(m_vs.nEffect > 4) VERIFY_INVALID(m_vs.nEffect, 4);

	if (m_vs.cTabSelType < CECHostPlayer::TSL_NORMAL) VERIFY_INVALID(m_vs.cTabSelType,CECHostPlayer::TSL_NORMAL);
	if (m_vs.cTabSelType > CECHostPlayer::TSL_PLAYER) VERIFY_INVALID(m_vs.cTabSelType,CECHostPlayer::TSL_PLAYER);

	//	Verify combo-skill ids
	VerifyComboSkills();
	
	// Verify combo-skill UI
	CECGameUIMan *pGameUIMan = g_pGame->GetGameRun()->GetUIManager()->GetInGameUIMan();
	if (pGameUIMan)
		pGameUIMan->VerifyComboSkillUI();
}

//	Verify combo-skill ids, this function will replace junior skills with
//	senior skills if condition met.
void CECConfigs::VerifyComboSkills()
{
	CECHostPlayer* pHost = g_pGame->GetGameRun()->GetHostPlayer();

	for (int j=0; j < EC_COMBOSKILL_NUM; j++)
	{
		if (!m_vs.comboSkill[j].nIcon)
			continue;

		for (int n=0; n < EC_COMBOSKILL_LEN; n++)
		{
			int idSkill = m_vs.comboSkill[j].idSkill[n];
			if (idSkill > 0)
			{
				CECSkill* pSkill = pHost->GetNormalSkill(idSkill, true);
				if (!pSkill)	pSkill = pHost->GetEquipSkillByID(idSkill);
				m_vs.comboSkill[j].idSkill[n] = pSkill ? pSkill->GetSkillID() : 0;
			}
		}
	}
}

AString CECConfigs::GetRandomThemeFile()
{
	AString strTheme;

	if (!m_strThemeFiles.empty())
	{
		int index = GetTickCount() % m_strThemeFiles.size();
		strTheme = "music\\" + m_strThemeFiles[index];
	}

	return strTheme;
}

void CECConfigs::SaveBlockedID()
{
	// create save folder
	AString strDir;
	strDir.Format("%s\\userdata\\blacklist", af_GetBaseDir());
	CreateDirectoryA(strDir, NULL);

	AString strFile;
	strFile.Format("%s\\%d.dat", strDir, g_pGame->GetGameRun()->GetHostPlayer()->GetCharacterID());
	FILE *file = fopen(strFile, "wb");
	if( !file )
	{
		a_LogOutput(1, "Cannot save the blacklist to %s", strFile);
		return;
	}
	
	fseek(file, 0, SEEK_END);
	
	memset(m_bs.idPlayer, 0, sizeof(m_bs.idPlayer));

	// save the size
	int listSize = m_BlockedArr.size();
	fwrite(&listSize, sizeof(int), 1, file);

	for(int i=0; i<listSize; i++)
	{
		int id = m_BlockedArr[i];
		if(i<EC_BLACKLIST_LEN)
		{
			m_bs.idPlayer[i] = id;
		}

		// save a id
		fwrite(&id, sizeof(int), 1, file);

		// save a name
		ACString strName = g_pGame->GetGameRun()->GetPlayerName(id, false);
		int strLen = strName.GetLength();
		fwrite(&strLen, sizeof(int), 1, file);
		if(strLen)
		{
			fwrite((const ACHAR*)strName, sizeof(ACHAR), strLen, file);
		}
	}
	
	fclose(file);
	a_LogOutput(1, "Blacklist saved to %s", strFile);
}

int CECConfigs::LoadBlockedList()
{
	m_BlockedArr.clear();

	//	Retrieve blacklist username.
	for (int i = 0; i < EC_BLACKLIST_LEN; i++)
	{
		int id = m_bs.idPlayer[i];
		if( id <= 0 ) break;

		m_BlockedArr.push_back(id);
	}

	// open the local block id list
	AString strFile;
	strFile.Format("%s\\userdata\\blacklist\\%d.dat", af_GetBaseDir(), g_pGame->GetGameRun()->GetHostPlayer()->GetCharacterID());
	FILE *file = fopen(strFile, "rb");
	if( file )
	{
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		if (fileSize != 0)
		{
			int listSize = 0;
			fread(&listSize, sizeof(int), 1, file);
			
			// read the file list
			for(int i=0; i<listSize; i++)
			{
				// read the id
				int id = 0;
				fread(&id, sizeof(int), 1, file);
				
				// read the name
				int strLen = 0;
				fread(&strLen, sizeof(int), 1, file);
				ACHAR* pData = (ACHAR*)a_malloctemp(sizeof(ACHAR)*(strLen+1));
				if(strLen > 0) fread(pData, sizeof(ACHAR), strLen, file);
				pData[strLen] = 0;
				ACString strName(pData);
				a_freetemp(pData);
				
				// merge the id list with the list from network
				if(!IsPlayerBlocked(id))
					m_BlockedArr.push_back(id);
				
				// insert this to name table
				if(!strName.IsEmpty())
				{
					g_pGame->GetGameRun()->AddPlayerName(id, strName);
				}
			}
		}
		fclose(file);
	}

	// check the unknown names
	abase::vector<int> aUnknowns;
	bool bGetAll = (a_Random(0, 9) == 4);
	for(unsigned int j=0; j<m_BlockedArr.size(); j++)
	{
		int id = m_BlockedArr[j];
		if(!g_pGame->GetGameRun()->GetPlayerName(id, false) || bGetAll)
		{
			aUnknowns.push_back(id);
		}
	}
	if(!aUnknowns.empty())
	{
		g_pGame->GetGameSession()->CacheGetPlayerBriefInfo(aUnknowns.size(), aUnknowns.begin(), 2);
	}

	return m_BlockedArr.size();
}

bool CECConfigs::IsPlayerBlocked(int idPlayer)
{
	abase::vector<int>::iterator iter = std::find(m_BlockedArr.begin(), m_BlockedArr.end(), idPlayer);
	return iter != m_BlockedArr.end();
}

void CECConfigs::SetBlockedName(int idPlayer, const ACString* pName)
{
	if( idPlayer <= 0 )
		return;

	bool bBlocked = IsPlayerBlocked(idPlayer);
	if(!pName || pName->IsEmpty())
	{
		if(bBlocked)
		{
			// delete from array
			unsigned int listSize = m_BlockedArr.size();
			for(unsigned int i=0;i<listSize; i++)
			{
				if(m_BlockedArr[i] == idPlayer)
				{
					if(i < listSize - 1)
					{
						memmove(m_BlockedArr.begin() + i, m_BlockedArr.begin() + i + 1,  sizeof(int) * (listSize - i - 1));
					}
					m_BlockedArr.pop_back();
					break;
				}
			}
		}
	}
	else
	{
		m_BlockedArr.push_back(idPlayer);
	}
}

void CECConfigs::SetOptimizeSettings(const EC_OPTIMIZE_SETTING& ops)
{
	// Save current gfx settings before copying new settings
	CECOptimize::GFX savedGfx = m_ops.gfx;
	
	// Copy all settings except gfx (we'll sync it separately)
	m_ops.bAutoSimplify = ops.bAutoSimplify;
	m_ops.iSimplifyMode = ops.iSimplifyMode;
	m_ops.nConfigMode = ops.nConfigMode;
	m_ops.nModelLimit = ops.nModelLimit;
	m_ops.nMateLimit = ops.nMateLimit;
	m_ops.nEnemyLimit = ops.nEnemyLimit;
	m_ops.nSpEffect = ops.nSpEffect;
	m_ops.nEffectsDistance = ops.nEffectsDistance;
	m_ops.nEffectsType = ops.nEffectsType;
	m_ops.nNameDistance = ops.nNameDistance;
	m_ops.bPeopleLimit = ops.bPeopleLimit;
	m_ops.bModelLimit = ops.bModelLimit;
	m_ops.bPeopleLimitByNear = ops.bPeopleLimitByNear;
	m_ops.bSpEffect = ops.bSpEffect;
	m_ops.bEffectsDistance = ops.bEffectsDistance;
	m_ops.bEffectsShow = ops.bEffectsShow;
	m_ops.bExcludeHost = ops.bExcludeHost;
	m_ops.bTargetSimpleModel = ops.bTargetSimpleModel;
	
	// Preserve existing gfx settings (except the ones we sync from other fields)
	m_ops.gfx = savedGfx;
	
	// Sync settings to m_ops.gfx before applying (and before saving)
	m_ops.gfx.bExcludeHost = m_ops.bExcludeHost;
	m_ops.gfx.nSpEffect = m_ops.nSpEffect;
	m_ops.gfx.bSpEffect = m_ops.bSpEffect;
	m_ops.gfx.nEffectsDistance = m_ops.nEffectsDistance;
	m_ops.gfx.bEffectsDistance = m_ops.bEffectsDistance;
	m_ops.gfx.nEffectsType = m_ops.nEffectsType;
	m_ops.gfx.bEffectsShow = m_ops.bEffectsShow;
	
	ApplyOptimizeSetting();
	
	// Save to INI file
	SaveSystemSettings();
}

void CECConfigs::ApplyOptimizeSetting()
{
	if( CECUIConfig::Instance().GetGameUI().bEnableOptimize )
	{
		// Sync bModelLimit from optimize settings to video settings
		// This ensures GetVideoSettings().bModelLimit returns the correct value
		m_vs.bModelLimit = m_ops.bModelLimit;
		
		// Sync bExcludeHost to m_ops.gfx.bExcludeHost before applying GFX settings
		m_ops.gfx.bExcludeHost = m_ops.bExcludeHost;
		// Sync effect settings to m_ops.gfx
		m_ops.gfx.nSpEffect = m_ops.nSpEffect;
		m_ops.gfx.bSpEffect = m_ops.bSpEffect;
		m_ops.gfx.nEffectsDistance = m_ops.nEffectsDistance;
		m_ops.gfx.bEffectsDistance = m_ops.bEffectsDistance;
		m_ops.gfx.nEffectsType = m_ops.nEffectsType;
		m_ops.gfx.bEffectsShow = m_ops.bEffectsShow;
		CECOptimize::Instance().SetGFX(m_ops.gfx);

		CECMemSimplify* pMemSimplify = g_pGame->GetGameRun()->GetMemSimplify();
		if (pMemSimplify)
		{
			pMemSimplify->SetAutoOptimize(m_ops.bAutoSimplify);
			if( !m_ops.bAutoSimplify )
				pMemSimplify->TransferUsageAll(m_ops.iSimplifyMode);
		}
		
		// Apply name distance setting (0-210 slider -> 0-210 meters)
		if (m_ops.nNameDistance > 0)
		{
			float fNameRadius = (float)m_ops.nNameDistance; // 0-210 -> 0-210 meters (1:1 mapping)
			m_fPlayerNameRenderRadius = fNameRadius;
			m_fNPCNameRenderRadius = fNameRadius * 0.5f; // NPC names at half distance
		}
		else
		{
			// Default values
			m_fPlayerNameRenderRadius = 60.0f;
			m_fNPCNameRenderRadius = 20.0f;
		}
		
		// Apply model render distance limit (0-210 slider -> 0-210 meters, direct mapping)
		if (m_ops.bModelLimit)
		{
			if (m_ops.nModelLimit > 0)
			{
				// Direct mapping: slider value = meters
				float fModelRadius = (float)m_ops.nModelLimit; // 0-210 -> 0-210 meters (1:1 mapping)
				// Allow values up to 210 meters (don't clamp to m_fSevActiveRad which is 213)
				// The server will handle objects beyond active radius anyway
				m_fModelRenderRadius = fModelRadius;
			}
			else
			{
				// If slider is at 0 and limit is enabled, use minimum radius
				m_fModelRenderRadius = m_fMinPVRadius;
			}
		}
		else
		{
			// Reset to default (full radius) when limit is disabled
			m_fModelRenderRadius = m_fSevActiveRad;
		}
		
		// All optimize settings are now integrated:
		// - nSpEffect, bSpEffect, nEffectsDistance, bEffectsDistance, nEffectsType, bEffectsShow:
		//   Applied to CECOptimize::GFX structure, can be checked via CanShowEffectByDistance() and CanShowEffectByType()
		// - bTargetSimpleModel: Applied in CECMemSimplify::ShouldExcludeFromSimplification() to exclude target from simplification
		// - nMateLimit, nEnemyLimit, bPeopleLimit, bPeopleLimitByNear: 
		//   Available via GetOptimizeSettings() or specific getters, can be checked in CECMemSimplify::ShouldLoadPlayer()
	}
}

bool CECConfigs::ShouldForceWaterRefract(){
	return g_pGame->GetGameRun()->GetWorld()->IsAtOboroMajorCity();
}
bool CECConfigs::IsUseWaterRefract(const EC_SYSTEM_SETTING &ss){
	return ss.bAdvancedWater;
}
bool CECConfigs::IsUseWaterRefract()const{
	return IsUseWaterRefract(m_ss);
}
bool CECConfigs::CanUseWaterRefract(){
	if (!g_pGame->GetGameRun() ||
		!g_pGame->GetGameRun()->GetWorld() ||
		!g_pGame->GetGameRun()->GetWorld()->GetTerrainWater()){
		return false;
	}
	if (!g_pGame->GetGameRun()->GetFullGlowRender()->CanDoFullGlow()){
		return false;
	}
	return true;
}
void CECConfigs::UseWaterRefract(EC_SYSTEM_SETTING &ss){
	ss.bAdvancedWater = true;
}
void CECConfigs::UseWaterRefract(){
	UseWaterRefract(m_ss);
	if (CanUseWaterRefract()){
		g_pGame->GetGameRun()->GetWorld()->GetTerrainWater()->SetExpensiveWaterFlag(m_ss.bAdvancedWater);
	}
}

bool CECConfigs::ShouldForceWaterReflect(){
	return g_pGame->GetGameRun()->GetWorld()->IsAtOboroMajorCity();
}
bool CECConfigs::IsUseWaterReflect(const EC_SYSTEM_SETTING &ss){
	return ss.nWaterEffect == 2;
}
bool CECConfigs::IsUseWaterReflect()const{
	return IsUseWaterReflect(m_ss);
}
bool CECConfigs::CanUseWaterReflect(){
	if (!g_pGame->GetGameRun() ||
		!g_pGame->GetGameRun()->GetWorld()){
		return false;
	}
	CECWorld *pWorld = g_pGame->GetGameRun()->GetWorld();
	if (!pWorld->GetTerrainWater() || !pWorld->GetTerrainWater()->CanRelfect()){
		return false;
	}
	return true;
}
void CECConfigs::UseWaterReflect(EC_SYSTEM_SETTING &ss){
	ss.nWaterEffect = 2;
}
void CECConfigs::UseWaterReflect(){
	UseWaterReflect(m_ss);
	if (CanUseWaterReflect()){
		g_pGame->GetGameRun()->GetWorld()->GetTerrainWater()->SetSimpleWaterFlag(!m_ss.nWaterEffect);
	}
}

void CECConfigs::ApplyWorldSpecificSetting(EC_SYSTEM_SETTING& ss)const{
	if (!IsUseWaterRefract(ss) && CanUseWaterRefract() && ShouldForceWaterRefract()){
		UseWaterRefract(ss);
	}
	if (!IsUseWaterReflect(ss) && CanUseWaterReflect() && ShouldForceWaterReflect()){
		UseWaterReflect(ss);
	}
}

void CECConfigs::ApplyWorldSpecificSetting(){
	if (m_bAdjusting){
		return;
	}
	m_bAdjusting = true;
	if (!IsUseWaterRefract() && CanUseWaterRefract() && ShouldForceWaterRefract()){
		UseWaterRefract();
	}
	if (!IsUseWaterReflect() && CanUseWaterReflect() && ShouldForceWaterReflect()){
		UseWaterReflect();
	}
	m_bAdjusting = false;
}