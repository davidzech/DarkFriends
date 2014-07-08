// DarkFriends.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "steam\steam_api.h"
#include "Log.h"
#include "DarkFriends.h"
#include "detours.h"

static char STEAM_FRIENDS[] = "SteamFriends";

typedef bool(__stdcall *SendP2PPacket_Ptr)(CSteamID, const void *, uint32, EP2PSend, int);
typedef bool(__stdcall *ReadP2PPacket_Ptr)(void *, uint32, uint32 *, CSteamID *, int);
 
static SendP2PPacket_Ptr originalSendP2PPacket = nullptr;
static ReadP2PPacket_Ptr originalReadP2PPacket = nullptr;

extern HMODULE steamHandle;

 __declspec(naked) bool __stdcall MySendP2PPacket(CSteamID steamIDRemote, const void *pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel)
{
	 void* This;
	 bool res;
	 __asm
	 {
			 push ebp
			 mov ebp, esp
			 sub esp, __LOCAL_SIZE
			 mov This, ecx
	 }
	 if (((ISteamFriends*(*)(void))GetProcAddress(steamHandle, STEAM_FRIENDS))()->GetFriendRelationship(steamIDRemote) == k_EFriendRelationshipFriend)
	 {
		 _asm {
				 push nChannel
				 push eP2PSendType
				 push cubData
				 push pubData
				 push dword ptr[ebp + 12]
				 push dword ptr[ebp + 8]
				 mov ecx, This				// restore just in case
				 call originalSendP2PPacket
				 mov res, al
		 }
	 }
	 else
	 {
		 res = false;
	 }
	_asm {
			mov ecx, This
			mov al, res;
			mov esp, ebp
			pop ebp
			retn 18h
	}
}


__declspec(naked) bool __stdcall MyReadP2PPacket(void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel)
{
	bool res;
	void *This;
	__asm
	{
			push ebp
			mov ebp, esp
			sub esp, __LOCAL_SIZE
			mov This, ecx
			push nChannel
			push psteamIDRemote
			push pcubMsgSize
			push cubDest
			push pubDest
			call originalReadP2PPacket
			mov res, al
	}
	if (((ISteamFriends*(*)(void))GetProcAddress(steamHandle, STEAM_FRIENDS))()->GetFriendRelationship(*psteamIDRemote) != k_EFriendRelationshipFriend)
	{
		res = false;
	}
	_asm {
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
	while (true)
	{
		steamHandle = GetModuleHandle(TEXT("steam_api.dll"));
		if (steamHandle == NULL)
			Sleep(100);
		else
			break;
	}

	ISteamNetworking* network = NULL;
	while (true)
	{
		network = ((ISteamNetworking*(*)(void))GetProcAddress(steamHandle, "SteamNetworking"))();
		if (network == NULL)
			Sleep(100);
		else
			break;
	} 

	originalSendP2PPacket = (SendP2PPacket_Ptr)(*(uint32**) network)[0]; // read vtable offset 0
	Log("Found SendP2PPacket at %p", originalSendP2PPacket);

	originalReadP2PPacket = (ReadP2PPacket_Ptr)(*(uint32**) network)[2];
	Log("Found ReadP2PPacket at %p", originalReadP2PPacket);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((PVOID*)&originalSendP2PPacket, (PVOID)MySendP2PPacket); //DetourAttach will modify the first variable to point to trampoline
	DetourAttach((PVOID*)&originalReadP2PPacket, (PVOID)MyReadP2PPacket);
	DetourTransactionCommit();

	Log("Functions hooked successfully");
}

void Destroy()
{
	DetourTransactionBegin();
	DetourDetach((PVOID*)&originalSendP2PPacket, (PVOID)MySendP2PPacket);
	DetourDetach((PVOID*)&originalReadP2PPacket, (PVOID)MyReadP2PPacket);
	DetourTransactionCommit();
	Log("Functions unhooked successfully");
}
