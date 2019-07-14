#include "stdafx.h"
#include "CustomJass.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <map>
#include <string>
using namespace std;

map <char * , DWORD> globalInt; //Key-Value
map <string , DWORD> globalArray; //ArrayName-ArrayAddress

//Loc=Location（表示需要一个目标）
//NoLoc=No Location（不需要一个目标）
//Ptr=Pointer（对象指针）
DWORD dwSendActionNaked = 0x6F33A850;
DWORD dwUseItemNoLoc = 0x6F33A6E0;
DWORD dwUseItemLoc = 0x6F33A740;
DWORD dwGetItemPtr = 0x6F3BF5D0;
DWORD dwGetWidgetPtr = 0x6F3BF030;

////////////////////////////////////////////
//注册JASS API
__declspec(naked) RegisterCustomJass_naked()
{
	__asm
	{
		SUB   ESP, 0x10C
		JMP	  DWORD PTR[dwRegJassRet]
	}
}

//注册
VOID RegisterCustomJassFunctions(DWORD address, char * name, char * types)
{
	__asm
	{
		LEA     EAX,  types
		PUSH    [EAX]
		MOV		ECX,  address
		LEA     EDX,  name
		MOV     EDX,  [EDX]
		CALL    RegisterCustomJass_naked
	}
}

//////////////////////////////////////////////////
//自定义Jass函数

//---1---
//看某个键是否被按下
BOOL JNATIVE KeyIsPressedMD(DWORD dwKey, char * unk)
{
	return (GetKeyState(dwKey) < 0);
}

//---2---
//在Jass中调用此函数写文件
VOID JNATIVE WriteFileStringMD(PJSTRING fileName, PJSTRING content, char * unk)
{
	FILE * fp = fopen(fileName->jstr1->str, "a");
	if (fp)
	{
		fprintf(fp, "%s\n", content->jstr1->str);
		fclose(fp);
	}
}

//---3---
//TempReplay.w3g是否存在，如存在，则游戏正在进行
BOOL JNATIVE IsTempReplayReadyMD(char * unk)
{
	return PathFileExistsA("TempReplay.w3g");
}

//---4---
//整数型的Hash表
DWORD JNATIVE GlobalGetIntMD(PJSTRING jStrKey, char *unk)
{
	char * strKey = jStrKey->jstr1->str;
	return globalInt[strKey];
}

//--5---
VOID  JNATIVE GlobalSetIntMD(PJSTRING jStrKey, DWORD value, char *unk)
{
	char * strKey = jStrKey->jstr1->str;
	globalInt[strKey] = value;
}

DWORD GetArrayValue(DWORD base, DWORD index)
{
	//base = ecx
	PDWORD pArray;
	__try
	{
		if (index < *(PDWORD)(base+8))
		{
			pArray = (PDWORD)(*(PDWORD)(base + 0xC));
			return pArray[index];
		}
		else
		{
			return 0;
		}
	}
	__except(1)
	{
		return 0;
	}
}

//--6--
//访问地图中定义的数组数据的值
//比如获取Dota中的补刀数，英雄的Unit等等
DWORD JNATIVE GetArrayDwordMD(PJSTRING arrayName, DWORD index, char *unk)
{
	char * str = arrayName->jstr1->str;
	string md = "MD"; //加上这个，减小冲突的概率
	md += str;
	DWORD base = globalArray[md];
	if (base == 0)
	{
		return 0;
	}
	return GetArrayValue(base, index);
}

//--7--
//将一个integer转成Unit
//在Jass中，Unit的存放方式是integer，但integer不能直接当成Unit使用
DWORD JNATIVE I2UMD(DWORD i, char * unk)
{
	return i;
}

//--8---
//执行基本的控制动作，比如移动(M)，停止(S)等等
VOID JNATIVE TargetOrderIssueMD(DWORD orderId, DWORD item, float x, float y, DWORD widget, DWORD option1, DWORD option2, char *unk)
{
	__try
	{
		__asm
		{
				PUSH  option2
				PUSH  option1
				MOV  ECX, widget
				MOV  EAX, dwGetWidgetPtr
				CALL EAX
				PUSH EAX //GetWidgetPointer
				MOV   EAX,  y
				PUSH  [EAX]
				MOV   EAX,  x
				PUSH  [EAX]
				MOV  ECX, item
				MOV  EAX, dwGetItemPtr
				CALL EAX
				PUSH EAX  //ItemPointer
				PUSH  orderId
				MOV   EAX, dwSendActionNaked
				CALL  EAX
		}
	}
	__except(1)
	{

	}
	
}

//---9---
//将Widget转化成Unit
//在Jass中Unit可以当成Widget使用，相反则不行
//所以自定义一个函数来把Widget转成Unit
//虽然类型不同，但值一样
DWORD JNATIVE Widget2UnitMD(DWORD src, char *unk)
{
	return src;
}



//---10---
//使用物品或技能（该物品或技能不需要选择一个地点使用，比如月女的跳）
VOID JNATIVE UseItemNoLocMD(DWORD orderId, DWORD item, DWORD opt1, DWORD opt2, char * unk)
{
	__try
	{
		__asm
		{
			PUSH opt2
			PUSH opt1
			MOV  ECX, item
			MOV  EAX, dwGetItemPtr
			CALL EAX
			PUSH EAX //GetItemPointer
			PUSH orderId
			MOV  EAX, dwUseItemNoLoc
			CALL EAX
		}
	}
	__except(1)
	{

	}
}


//---11---
//使用物品或技能（该物品或技能必须选择一个地点使用，比如敌法的跳）
VOID JNATIVE UseItemLocMD(DWORD orderId, DWORD item, float x, float y, DWORD opt1, DWORD opt2, char *unk)
{
	__try
	{
		__asm
		{
			PUSH  opt2
			PUSH  opt1
			MOV   EAX,  y
			PUSH  [EAX]
			MOV   EAX,  x
			PUSH  [EAX]
			MOV   ECX, item
			MOV   EAX, dwGetItemPtr
			CALL  EAX
			PUSH  EAX
			PUSH  orderId
			MOV   EAX, dwUseItemLoc
			CALL  EAX
		}
	}
	__except(1)
	{

	}
}

//---12---
//清除全局的数据
//每一局游戏结束后，将上一局的数据清空，以防出错
VOID JNATIVE ClearArrayDataMD(char *unk)
{
	globalInt.clear();
	globalArray.clear();
}
	

//将common2.j中用到的一些自定义函数向JASS引擎注册
//jass中所有函数在使用之前都必须向JASS引擎注册
VOID __stdcall RegisterAllCustomJassFunctions()
{
	RegisterCustomJassFunctions((DWORD)KeyIsPressedMD, "KeyIsPressedMD", "(I)B");
	RegisterCustomJassFunctions((DWORD)WriteFileStringMD, "WriteFileStringMD", "(SS)V");
	RegisterCustomJassFunctions((DWORD)IsTempReplayReadyMD, "IsTempReplayReadyMD", "()B");
	RegisterCustomJassFunctions((DWORD)GlobalGetIntMD, "GlobalGetIntMD", "(S)I");
	RegisterCustomJassFunctions((DWORD)GlobalSetIntMD, "GlobalSetIntMD", "(SI)V");
	RegisterCustomJassFunctions((DWORD)GetArrayDwordMD, "GetArrayDwordMD", "(SI)I");
	RegisterCustomJassFunctions((DWORD)I2UMD, "I2UMD", "(I)Hunit;");
	RegisterCustomJassFunctions((DWORD)TargetOrderIssueMD, "TargetOrderIssueMD", "(IHitem;RRHwidget;II)V");
	RegisterCustomJassFunctions((DWORD)Widget2UnitMD, "Widget2UnitMD", "(Hwidget;)Hunit;");
	RegisterCustomJassFunctions((DWORD)UseItemNoLocMD, "UseItemNoLocMD", "(IHitem;II)V");
	RegisterCustomJassFunctions((DWORD)UseItemLocMD, "UseItemLocMD", "(IHitem;RRII)V");
	RegisterCustomJassFunctions((DWORD)ClearArrayDataMD, "ClearArrayDataMD", "()V");
}




//要记录的数组
char targetArray[] = "A L1 ";

//================================================================
//war3map.j中的数组
//================================================================
VOID __stdcall DealWithArrayAddress(DWORD ebx, DWORD ecx)
{
	//ECX地址
	//EBX名称

	char * str = (char *)ebx;
	string md = "MD";
	if (strstr(targetArray, str))	//比较数组名字
	{
		globalArray[md + str] = ecx;
	//	char buffer[256];
	//	sprintf(buffer, "%s:0x%X", str, globalArray[md+str]);
	//	OutputDebugString(buffer);
	}	
}

__declspec(naked) VOID GetArrayAddress(VOID)
{
	__asm
	{
			PUSHAD
			PUSH  ECX
			PUSH  EBX
			CALL  DealWithArrayAddress
			POPAD
			
			MOV   EAX,  DWORD PTR[ESP+8]
			CMP   EAX,  DWORD PTR[ECX+8]
			RETN
	}
}


//
//挂钩获取jass引擎中的array类型的数据，即数组数据
//Dota会将一些重要的数据放入全局数组中，比如补刀数，英雄单位的Unit（类似于句柄）等等
DWORD WINAPI thread_Hook_JassArray(LPVOID lp)
{
	while (1)
	{
		if (GetModuleHandle("game.dll"))
		{
			break;
		}
		Sleep(500);
	}
	DWORD dwBase = (DWORD)GetModuleHandle("game.dll");
	BYTE CallCodes[7] = {0xE8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90};
	DWORD dwHookAddr = 0x45A1D0 + dwBase;
	
	DWORD tmp = (DWORD)GetArrayAddress - dwHookAddr - 5;
	memcpy(CallCodes+1, &tmp, 4);
	if (!WriteProcessMemory((HANDLE)-1, (PVOID)dwHookAddr, CallCodes, 7, &tmp))
	{
		OutputDebugString("hook erro2");
		return FALSE;
	}
	
	dwHookAddr = dwBase + 0x45A1B0;
	tmp = (DWORD)GetArrayAddress - dwHookAddr - 5;
	memcpy(CallCodes+1, &tmp, 4);
	if (!WriteProcessMemory((HANDLE)-1, (PVOID)dwHookAddr, CallCodes, 7, &tmp))
	{
		OutputDebugString("hook erro3");
		return FALSE;
	}
	
	OutputDebugString("[JassArray]: Hook ok");
	return TRUE;
}