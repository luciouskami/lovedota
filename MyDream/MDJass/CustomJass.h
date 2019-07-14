

#define JNATIVE	__stdcall

typedef struct  JSTRING1_TAG
{
	DWORD unk[7];
	char  * str;
}JSTRING1, *PJSTRING1;

typedef struct  JSTRING_TAG
{
	DWORD unk1;
	DWORD unk2;
	PJSTRING1 jstr1;
}JSTRING, *PJSTRING;

VOID __stdcall RegisterAllCustomJassFunctions();


DWORD WINAPI thread_Hook_JassArray(LPVOID lp);