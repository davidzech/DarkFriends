// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DarkFriends.h"
#include "Log.h"
#include <string>

HMODULE steamHandle;
HMODULE thisModule;
const char* logFilePath = nullptr;
HANDLE hFile;

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
			logFilePath = dll.c_str();
			hFile = CreateFile(logFilePath, FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			Log("DarkFriends Attached");
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

