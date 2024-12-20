// HackAnyGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "proc.h"
#include "mem.h"

static int leet_ammo()
{
	//Get ProcID of the target process
	DWORD procID = GetProcID(L"ac_client.exe");

	//Getmodulebaseaddress
	uintptr_t moduleBase = GetModuleBaseAddress(procID, L"ac_client.exe");

	//Get Handle to Process
	HANDLE hProcess = 0;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);

	//Resolve base address of the pointer chain
	uintptr_t dynamicPtrBaseAddr = moduleBase + 0x10f4f4;

	std::cout << "DynamicPtrBaseAddr = " << "0x" << std::hex << dynamicPtrBaseAddr << std::endl;

	//Resolve our ammo pointer chain
	std::vector<unsigned int> ammoOffsets = { 0x374, 0x14, 0x0 };
	uintptr_t ammoAddr = FindDMAAddy(hProcess, dynamicPtrBaseAddr, ammoOffsets);

	std::cout << "ammoAddr = " << "0x" << std::hex << ammoAddr << std::endl;

	//Read Ammo value
	int ammoValue = 0;

	ReadProcessMemory(hProcess, (BYTE*)ammoAddr, &ammoValue, sizeof(ammoValue), nullptr);
	std::cout << "Current ammo = " << std::dec << ammoValue << std::endl;

	//Write to it
	int newAmmo = 1337;
	WriteProcessMemory(hProcess, (BYTE*)ammoAddr, &newAmmo, sizeof(newAmmo), nullptr);

	//Read out again
	ReadProcessMemory(hProcess, (BYTE*)ammoAddr, &ammoValue, sizeof(ammoValue), nullptr);

	std::cout << "New ammo = " << std::dec << ammoValue << std::endl;


	getchar();
	return 0;
}

static int recoil_delete()
{
	HANDLE hProcess = 0;

	uintptr_t moduleBase = 0, localPlayerPtr = 0, healthAddr = 0;
	bool bHealth = false, bAmmo = false, bRecoil = false, bGrenade = false;

	const int newValue = 1337;

	DWORD procID = GetProcID(L"ac_client.exe");

	if (procID)
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);

		moduleBase = GetModuleBaseAddress(procID, L"ac_client.exe");

		localPlayerPtr = moduleBase + 0x10f4f4;

		healthAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0xf8 });

	}
	
	else
	{
		std::cout << "Process not found, press enter to exit\n";
		getchar();
		return 0;
	}

	DWORD dwExit = 0;

	while (GetExitCodeProcess(hProcess, &dwExit) && dwExit == STILL_ACTIVE)
	{
		if (GetAsyncKeyState(VK_F1) & 1)
		{
			bHealth = !bHealth;
		}
		if (GetAsyncKeyState(VK_F2) & 1)
		{
			bAmmo = !bAmmo;

			if (bAmmo)
			{
				//ff 06 = inc [esi];
				mem::PatchEx((BYTE*)(moduleBase + 0x637e9), (BYTE*)"\xFF\x06", 2, hProcess);
			}
			else
			{
				//ff 0e = dec [esi];
				mem::PatchEx((BYTE*)(moduleBase + 0x637e9), (BYTE*)"\xFF\x0E", 2, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_F3) & 1)
		{
			bRecoil = !bRecoil;

			if (bRecoil)
			{
				mem::NopEx((BYTE*)(moduleBase + 0x63786), 10, hProcess);
			}
			else
			{
				mem::PatchEx((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x8B\xCE\xFF\xD2", 10, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_F4) & 1)
		{
			bGrenade = !bGrenade;

			if (bGrenade)
			{
				mem::NopEx((BYTE*)(moduleBase + 0x63378), 2, hProcess);
			}
			else
			{
				mem::PatchEx((BYTE*)(moduleBase = 0x63378), (BYTE*)"\xFF\x08", 2, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_F12) & 1)
		{
			return 0;
		}

		// continuous write or freeze
		if (bHealth)
		{
			mem::PatchEx((BYTE*)healthAddr, (BYTE*)&newValue, sizeof(newValue), hProcess);
		}

		Sleep(10);
	}

	std::cout << "Process not found, press enter to exit\n";
	getchar();
	return 0;
}

int main()
{
	leet_ammo();
	recoil_delete();
}

