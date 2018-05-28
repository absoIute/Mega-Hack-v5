#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>
#include <filesystem>
#include <sstream>

#include <Windows.h>

std::vector<std::string> v;
char *dir;

void addToVector(std::string str)
{
	v.push_back(str.substr(str.find_last_of("\\", 0x7FFFFFFF)));
}

void refreshdir()
{
	v.clear();
	for (auto &p : std::experimental::filesystem::directory_iterator("Resources\\deathSounds\\"))
	{
		addToVector(p.path().string());
	}
}

void setup()
{
	refreshdir();

	if (v.size() != 0)
	{
		dir = new char[MAX_PATH];
		DWORD ptr = (DWORD)(dir), addr = (DWORD)GetModuleHandleA("GeometryDash.exe") + 0x20A5AC, old;

		VirtualProtect((void*)addr, MAX_PATH, PAGE_EXECUTE_READWRITE, &old);
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)(addr), &ptr, 4, NULL);

		strcpy(dir, "deathSounds");
	}
}

void randomiseDeathSound()
{
	if (v.size())
	{
		refreshdir();
		strcpy((char*)(dir + 11), v[rand() % v.size()].c_str());
	}
	else
	{
		setup();
	}
}

bool direxist(const char *szPath)
{
	DWORD dwAttrib = GetFileAttributesA(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	if (!direxist("Resources\\deathSounds"))
	{
		CreateDirectoryA("Resources\\deathSounds", NULL);
	}

	for (;;)
	{
		randomiseDeathSound();
		Sleep(500);
	}

	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(0, 0x1000, &MainThread, 0, 0, NULL);
	}
	return TRUE;
}