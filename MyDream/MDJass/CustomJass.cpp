#include "stdafx.h"
#include "CustomJass.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <map>
#include <string>
using namespace std;

map <char * , DWORD> globalInt; //Key-Value
map <string , DWORD> globalArray; //ArrayName-ArrayAddress

//Loc=Location����ʾ��Ҫһ��Ŀ�꣩
//NoLoc=No Location������Ҫһ��Ŀ�꣩
//Ptr=Pointer������ָ�룩
DWORD dwSendActionNaked = 0x6F33A850;
DWORD dwUseItemNoLoc = 0x6F33A6E0;
DWORD dwUseItemLoc = 0x6F33A740;
DWORD dwGetItemPtr = 0x6F3BF5D0;
DWORD dwGetWidgetPtr = 0x6F3BF030;

////////////////////////////////////////////
//ע��JASS API
__declspec(naked) RegisterCustomJass_naked()
{
	__asm
	{
		SUB   ESP, 0x10C
		JMP	  DWORD PTR[dwRegJassRet]
	}
}

//ע��
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
//�Զ���Jass����

//---1---
//��ĳ�����Ƿ񱻰���
BOOL JNATIVE KeyIsPressedMD(DWORD dwKey, char * unk)
{
	return (GetKeyState(dwKey) < 0);
}

//---2---
//��Jass�е��ô˺���д�ļ�
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
//TempReplay.w3g�Ƿ���ڣ�����ڣ�����Ϸ���ڽ���
BOOL JNATIVE IsTempReplayReadyMD(char * unk)
{
	return PathFileExistsA("TempReplay.w3g");
}

//---4---
//�����͵�Hash��
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
//���ʵ�ͼ�ж�����������ݵ�ֵ
//�����ȡDota�еĲ�������Ӣ�۵�Unit�ȵ�
DWORD JNATIVE GetArrayDwordMD(PJSTRING arrayName, DWORD index, char *unk)
{
	char * str = arrayName->jstr1->str;
	string md = "MD"; //�����������С��ͻ�ĸ���
	md += str;
	DWORD base = globalArray[md];
	if (base == 0)
	{
		return 0;
	}
	return GetArrayValue(base, index);
}

//--7--
//��һ��integerת��Unit
//��Jass�У�Unit�Ĵ�ŷ�ʽ��integer����integer����ֱ�ӵ���Unitʹ��
DWORD JNATIVE I2UMD(DWORD i, char * unk)
{
	return i;
}

//--8---
//ִ�л����Ŀ��ƶ����������ƶ�(M)��ֹͣ(S)�ȵ�
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
//��Widgetת����Unit
//��Jass��Unit���Ե���Widgetʹ�ã��෴����
//�����Զ���һ����������Widgetת��Unit
//��Ȼ���Ͳ�ͬ����ֵһ��
DWORD JNATIVE Widget2UnitMD(DWORD src, char *unk)
{
	return src;
}



//---10---
//ʹ����Ʒ���ܣ�����Ʒ���ܲ���Ҫѡ��һ���ص�ʹ�ã�������Ů������
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
//ʹ����Ʒ���ܣ�����Ʒ���ܱ���ѡ��һ���ص�ʹ�ã�����з�������
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
//���ȫ�ֵ�����
//ÿһ����Ϸ�����󣬽���һ�ֵ�������գ��Է�����
VOID JNATIVE ClearArrayDataMD(char *unk)
{
	globalInt.clear();
	globalArray.clear();
}
	

//��common2.j���õ���һЩ�Զ��庯����JASS����ע��
//jass�����к�����ʹ��֮ǰ��������JASS����ע��
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




//Ҫ��¼������
char targetArray[] = "A L1 ";

//================================================================
//war3map.j�е�����
//================================================================
VOID __stdcall DealWithArrayAddress(DWORD ebx, DWORD ecx)
{
	//ECX��ַ
	//EBX����

	char * str = (char *)ebx;
	string md = "MD";
	if (strstr(targetArray, str))	//�Ƚ���������
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
//�ҹ���ȡjass�����е�array���͵����ݣ�����������
//Dota�ὫһЩ��Ҫ�����ݷ���ȫ�������У����粹������Ӣ�۵�λ��Unit�������ھ�����ȵ�
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