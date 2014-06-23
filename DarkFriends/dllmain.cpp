// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DarkFriends.h"
#include "Log.h"
#include <string>

HMODULE steamHandle;
HMODULE thisModule;
FILE *logOut = nullptr;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			char fileName[MAX_PATH];
			GetModuleFileName(hModule, fileName, MAX_PATH);
			std::string dll = fileName;
			dll += ".log";
			if (fopen_s(&logOut, dll.c_str(), "a+") != 0)
			{
				logOut = stdout;
			}
			Log("Attached");
			DisableThreadLibraryCalls(hModule);
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Bootstrap, hModule, 0, NULL);
			break;
		}
		case DLL_PROCESS_DETACH:
			Destroy();
			break;
	}
	return TRUE;
}

