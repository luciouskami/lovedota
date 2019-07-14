// MDJass.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CustomJass.h"

char nameList[100][40] = {"DisplayTimedTextToPlayer", "TimerStart", "DestroyTimer", "CreateUnit", "SetUnitOwner", \
							"GetEventPlayerChatString", "UnitRemoveAbility", "AddSpecialEffectTarget", "CreateUnitAtLoc", "UnitAddAbility", \
							"GetSpellAbilityId", "GetTriggerUnit"};
int  nameNumber = 12;	//挂钩12个JASS函数
char * common2 = "common2.j"; //将War3x.mpq!common.j替换成 common2.j
char * myMpq = "C:\\Documents and Settings\\MentalDease\\My Documents\\Crack\\DreamDota\\MyDream\\MDJass.mpq";	//mpq文件中包含自己的common2.j
DWORD dwRegJassHookAddr = 0x6F455BC0;
DWORD dwSetNewHashAddr = 0x6F3B2520;
BYTE  setNewHashBytes[] = {0xB8, 0xF8, 0x6C, 0xEA, 0x32};


//处理后的函数名(在要HOOK的函数后加MD)
char newName[100] = {0};

char * __stdcall GetNewName(char * oldName)
{

	RegisterAllCustomJassFunctions();
	strcpy(newName, oldName);
	for (int i=0; i< nameNumber; i++)
	{
		if (stricmp(oldName, nameList[i]) == 0)
		{
			strcat(newName, "MD");
			break;
		}
	}
	return newName;
}


DWORD dwRegJassRet;
__declspec(naked) HeaderRegJassFunc()
{
	__asm
	{
		PUSHAD
		PUSH   EDX
		CALL   GetNewName
		POPAD
		LEA   EDX, newName
		SUB   ESP, 0x10C
		JMP	  DWORD PTR[dwRegJassRet]		
	}
}


typedef BOOL (__stdcall * SFileOpenArchive_type)(char *archivename, DWORD dwPriority, DWORD dwFlags, HANDLE *handle);

SFileOpenArchive_type SFileOpenArchive = NULL;
HANDLE hMpq;

DWORD WINAPI thread_Hook(LPVOID lp)
{
	DWORD dwGameDllBase = (DWORD)GetModuleHandle("Game.dll");
	DWORD dwStormDllBase = (DWORD)GetModuleHandle("Storm.dll");
	DWORD tmp;

	while (!(dwGameDllBase && dwStormDllBase))
	{
		//等待game.dll和Storm.dll加载完成
		dwGameDllBase = (DWORD)GetModuleHandle("Game.dll");
		dwStormDllBase = (DWORD)GetModuleHandle("Storm.dll");
		Sleep(100);
	}
	

	//加载MDJass.mpq
	SFileOpenArchive = (SFileOpenArchive_type)GetProcAddress((HMODULE)dwStormDllBase, (LPCTSTR)266);
	if (SFileOpenArchive == NULL)
	{
		OutputDebugString("getprocaddress error");
		return FALSE;
	}

	if(!SFileOpenArchive(myMpq, 0xA, 6, &hMpq))
	{
		OutputDebugString("sfileOpenArchive error");
	//	return FALSE;
	}
	
	DWORD dwCommon2Addr = (DWORD)common2;
	if (!WriteProcessMemory((HANDLE)-1, (PVOID)(dwGameDllBase+ 0x3B24FC), &dwCommon2Addr, 4, &tmp))
	{
		OutputDebugString("modify common2 error");
		return FALSE;
	}

	dwRegJassRet = dwRegJassHookAddr + 6;
	BYTE  JmpBytes[6] = {0xE9, 0x00, 0x00, 0x00, 0x00, 0x90};
	tmp = (DWORD)HeaderRegJassFunc - dwRegJassHookAddr - 5;
	memcpy(JmpBytes+1, &tmp, 4);
	if (!WriteProcessMemory((HANDLE)-1, (PVOID)dwRegJassHookAddr, JmpBytes, 6, &tmp))
	{
		OutputDebugString("Hook error");
		return FALSE;
	}
	
	//修正魔兽对common2.j的校验值使之与common.j的校验值相同
	//否则进别人主机需要重新下图。。。
	if (!WriteProcessMemory((HANDLE)-1, (PVOID)dwSetNewHashAddr, setNewHashBytes, 5, &tmp))
	{
		OutputDebugString("setnewhash error");
		return FALSE;
	}

	OutputDebugString("hook ok");
	return TRUE;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_Hook, NULL, 0, NULL);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_Hook_JassArray, NULL, 0, NULL);
	}
    return TRUE;
}

