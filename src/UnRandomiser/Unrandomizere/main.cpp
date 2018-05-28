#include <Windows.h>
#include <string>
#include <sstream>
#include <iomanip>

//types: 0 - add, 1 - sub, 2 - xor

bool hooked = false;
unsigned long value, difference, type;

void hook_point(void *toBeHooked, void *hooker)
{
	DWORD oldProtect, newProtect;
	DWORD offset = (DWORD)hooker - (DWORD)toBeHooked - 5;
	VirtualProtect((LPVOID)toBeHooked, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)toBeHooked, new byte{ 0xe9 }, 1, NULL);
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)((DWORD)toBeHooked + 1), &offset, 4, NULL);
	VirtualProtect((LPVOID)toBeHooked, 5, oldProtect, &newProtect);
}

int __declspec(naked) hook()
{
	switch (type)
	{
	case 0:
		value += difference;
		break;
	case 1:
		value -= difference;
		break;
	}

	value &= 0x7FFF;

	__asm mov eax, value
	__asm retn
}

void __declspec(dllexport) hooker(unsigned long *param)
{
	value = *param;
	++param;
	difference = *param;
	++param;
	type = *param;
	++param;

	if (!hooked)
	{
		while (*param)
		{
			hook_point(reinterpret_cast<void*>(*param), &hook);
			++param;
		}
	}
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(NULL, 0x1000, &MainThread, NULL, 0, NULL);
	}
	return TRUE;
}