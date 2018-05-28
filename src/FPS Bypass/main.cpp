#include <windows.h>
#include <iostream>
 
typedef void*   (__cdecl *fSharedApplication)();
typedef void    (__thiscall *fSetAnimationInterval)(void *instance, double delay);
fSharedApplication sharedApplication;
fSetAnimationInterval setAnimInterval;
float interval = 0; //this is changed externally
 
DWORD WINAPI Main_Thread(LPVOID lpParam)
{
    interval = 1.0f / 60;
 
    HMODULE hMod = LoadLibrary(L"libcocos2d.dll");
    sharedApplication = (fSharedApplication)GetProcAddress(hMod, "?sharedApplication@CCApplication@cocos2d@@SAPAV12@XZ");
    setAnimInterval = (fSetAnimationInterval)GetProcAddress(hMod, "?setAnimationInterval@CCApplication@cocos2d@@UAEXN@Z");
 
    void *application = sharedApplication();
 
    while (1)
	{
        setAnimInterval(application, interval);
        Sleep(500);
    }
}
 
BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
        CreateThread(0, 0x1000, &Main_Thread, 0, 0, NULL);
    return TRUE;
}