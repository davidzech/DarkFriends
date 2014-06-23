// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DarkFriends.h"
#include "Log.h"
#include <detours.h>

HMODULE steamHandle;
HMODULE thisModule;
bool bootstrapped = false;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	{
		if (!bootstrapped)
		{
			Log("Attached");
			bootstrapped = true;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Bootstrap, hModule, 0, NULL);
		}

		break;
	}
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		Destroy();
		break;
	}
	return TRUE;
}

