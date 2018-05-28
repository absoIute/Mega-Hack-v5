#include <Windows.h>
#include <cstdint>
#include <string>

static uint32_t offsets[10] = { 0x1E8, 0x1F4, 0x200, 0x20C, 0x218, 0x224, 0x230, 0x23C, 0x248, 0x254 };
static uint32_t maxvals[10] = { 142, 51, 43, 35, 35, 26, 17, 41, 41, 7 };

static bool on = true;

void __declspec(dllexport) toggle(void *param)
{
	on = (param == nullptr);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	srand(GetCurrentTime());

	uint32_t base = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(GetModuleHandleA("GeometryDash.exe")) + 0x3222D0);

	for (;;)
	{
		if (on)
			for (auto i = 0; i < 10; ++i)
			{
				*reinterpret_cast<uint32_t*>(base + offsets[i]) = rand() % maxvals[i] + (maxvals[i] != 41);
				*reinterpret_cast<uint32_t*>(base + offsets[i] + 4) = 0;
				Sleep(50);
			}
		else
			Sleep(50);
	}

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0x1000, &MainThread, NULL, 0, NULL);
	return TRUE;
}