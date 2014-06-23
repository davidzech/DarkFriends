// DarkFriends.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "steam\steam_api.h"
#include "Log.h"
#include "DarkFriends.h"
#include <detours.h>

typedef bool(__stdcall *SendP2PPacket_Ptr)(CSteamID, const void *, uint32, EP2PSend, int);
typedef bool(__stdcall *ReadP2PPacket_Ptr)(void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel);
 
static SendP2PPacket_Ptr originalSendP2PPacket = nullptr;
static ReadP2PPacket_Ptr originalReadP2PPacket = nullptr;

extern HMODULE steamHandle;

 __declspec(naked) bool __stdcall MySendP2PPacket(CSteamID steamIDRemote, const void *pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel)
{
	// bool res;
	 void * This;
	 ISteamFriends *friends;
	 bool res;
	_asm
	{
		push ebp
		mov ebp, esp
		sub esp, __LOCAL_SIZE
		mov This, ecx
	}
	friends = ((ISteamFriends*(*)(void))GetProcAddress(steamHandle, "SteamFriends")) ();
	if (friends->GetFriendRelationship(steamIDRemote) == k_EFriendRelationshipFriend)
	{
		_asm
		{
			push nChannel
			push eP2PSendType
			push cubData
			push pubData
			push dword ptr[ebp + 12]
			push dword ptr[ebp + 8]
			mov ecx, This
			call originalSendP2PPacket
			mov res, al
		}
	}
	else
	{
		res = false;
	}

	_asm
	{
		mov ecx, This
		mov al, res
		mov esp, ebp
		pop ebp
		retn 18h
	}
}


__declspec(naked) bool __stdcall MyReadP2PPacket(void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel)
{
		ISteamFriends *friends;
		bool res;
		void *This;
		_asm
		{
			push ebp
			mov ebp, esp
			sub esp, __LOCAL_SIZE
			mov This, ecx
		}
		_asm
		{
				push nChannel
				push psteamIDRemote
				push pcubMsgSize
				push cubDest
				push pubDest
				mov ecx, This
				call originalReadP2PPacket
				mov res, al
		}
		friends = ((ISteamFriends*(*)(void))GetProcAddress(steamHandle, "SteamFriends")) ();
		if (friends->GetFriendRelationship(*psteamIDRemote) != k_EFriendRelationshipFriend)
		{
			res = false;
		}
		_asm
		{
				mov ecx, This
				mov al, res
				mov esp, ebp
				pop ebp
				retn 20
		}
}
void Bootstrap(HMODULE dll)
{

	HMODULE baseAddress = GetModuleHandle(NULL);

	Log("Base is %p", baseAddress);

	steamHandle = NULL;
	while (steamHandle == NULL)
	{
		steamHandle = GetModuleHandle(TEXT("steam_api.dll"));
		Sleep(100);
	}

	//void *addr = (void*) ((uintptr_t) baseAddress + 0x1000 + p2poffset);
	ISteamNetworking* network = NULL;
	do
	{
		network = ((ISteamNetworking*(*)(void))GetProcAddress(steamHandle, "SteamNetworking"))();
	} while (network == NULL);
	void *addr = (*(uint32***) network)[0];
	Log("Wrote to addr %p", addr);

	void *addr2 = (*(uint32***) network)[2];

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((PVOID*)&addr, (PVOID*)MySendP2PPacket);
	DetourAttach((PVOID*)&addr2, (PVOID) MyReadP2PPacket);
	DetourTransactionCommit();

	Log("Wrote to addr %p", addr);
	originalSendP2PPacket = (SendP2PPacket_Ptr) addr;
	originalReadP2PPacket = (ReadP2PPacket_Ptr) addr2;

}

void Destroy()
{

}
