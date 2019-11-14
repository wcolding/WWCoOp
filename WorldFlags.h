#pragma once
#include "WWCore.h"
#include "DolphinHook.h"

#define WW_LOCAL_FLAGS 0x003C5382
#define WW_KEYS_OFFSET 0x1E

struct WorldFlag
{
	unsigned int address;
	__int8 flag;
};

WorldFlag DinsPearlPlaced = { 0x003C5240, 0b10000000 };
WorldFlag FaroresPearlPlaced = { 0x003C5240, 0b01000000 };
WorldFlag NayrusPearlPlaced = { 0x003C5240, 0b00010000 };

WorldFlag TotGRaised = { 0x003C524A, 0b01000000 };

WorldFlag TingleIsFree			= { 0x003C5237, 0b10000000 }; // Cannot redeem statue rewards if this isn't set
WorldFlag DragonTingleStatue	= { 0x003C5296, 0b00000100 };
WorldFlag ForbiddenTingleStatue	= { 0x003C5296, 0b00001000 };
WorldFlag GoddessTingleStatue	= { 0x003C5296, 0b00010000 };
WorldFlag EarthTingleStatue		= { 0x003C5296, 0b00100000 };
WorldFlag WindTingleStatue		= { 0x003C5296, 0b01000000 };

WorldFlag GreatFairyGift1		= { 0x003C525C, 0b00100000 }; // Northern Fairy Island
WorldFlag GreatFairyGift2		= { 0x003C525C, 0b00001000 }; // Eastern Fairy Island
WorldFlag GreatFairyGift3		= { 0x003C525C, 0b00000010 }; // Western Fairy Island
WorldFlag GreatFairyGift4		= { 0x003C525C, 0b00010000 }; // Outset Island
WorldFlag GreatFairyGift5		= { 0x003C525C, 0b00000001 }; // Thorned Fairy Island
WorldFlag GreatFairyGift6		= { 0x003C525C, 0b00000100 }; // Southern Fairy Island

// Describes a set of stages with shared flags, such as for islands or dungeons
struct WorldGroup
{
	unsigned int permAddress;
	vector <string> stageNames;

	bool ContainsStage(string stage)
	{
		for (int i = 0; i < stageNames.size(); i++)
		{
			if (stageNames[i] == stage)
				return true;
		}
		return false;
	}
};

WorldGroup wg_OutsetInterior = { 0x003C5116, {"LinkUG", "Omasao", "A_mori"} }; // This address gets reused as the last non-permanent flags in readable memory? Maybe? idk yet

WorldGroup wg_DRC	= { 0x003C4FF6, {"M_NewD2", "M_Dra09", "M_DragB"} };
WorldGroup wg_FW	= { 0x003C501A, {"kindan", "kinMB", "kinBOSS"} };
WorldGroup wg_TotG	= { 0x003C503E, {"Siren", "SirenMB", "SirenB"} };
WorldGroup wg_Earth = { 0x003C5062, {"M_Dai", "M_DaiMB", "M_DaiB"} };
WorldGroup wg_Wind	= { 0x003C5086, {"kaze", "kazeMB", "kazeB"} }; 

struct WorldGroupFlag
{
	WorldGroup group;
	unsigned int offset;
	__int8 flag;
};

// We probably won't individually address these but it's still useful information
WorldGroupFlag OutsetLinksHouseChest	= { wg_OutsetInterior, 1, 0b00100000 };
WorldGroupFlag OutsetGrasscutterChest	= { wg_OutsetInterior, 1, 0b00010000 };
WorldGroupFlag OutsetFairyBoulder		= { wg_OutsetInterior, 4, 0b00010000 };

bool IsFlagSet(WorldFlag flag)
{
	char buffer[1];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + flag.address), &buffer, 1, nullptr);
	return (buffer[0] & flag.flag) != 0 ? true : false;
}

bool IsFlagSet(WorldGroupFlag flag)
{
	char buffer[1];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + flag.group.permAddress + flag.offset), &buffer, 1, nullptr);
	return (buffer[0] & flag.flag) != 0 ? true : false;
}

void SetFlag(WorldFlag flag)
{
	char buffer[1];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + flag.address), &buffer, 1, nullptr);
	buffer[0] = buffer[0] | flag.flag;
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + flag.address), &buffer, 1, nullptr);
}

void SetFlag(WorldGroupFlag flag)
{
	char buffer[1];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + flag.group.permAddress + flag.offset), &buffer, 1, nullptr);
	buffer[0] = buffer[0] | flag.flag;
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + flag.group.permAddress + flag.offset), &buffer, 1, nullptr);
}

__int8 GetFlag(WorldFlag flag)
{
	char buffer[1];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + flag.address), &buffer, 1, nullptr);
	return buffer[0];
}

__int8 GetFlag(WorldGroupFlag flag)
{
	char buffer[1];
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + flag.group.permAddress + flag.offset), &buffer, 1, nullptr);
	return buffer[0];
}

// This will expand as we sync more flags
struct WWFlags
{
	__int8 TowerRaised = 0;
	__int8 TingleFree = 0;
	__int8 TingleStatues = 0;
	__int8 GreatFairies = 0;
	__int8 DRCFlags[10];
	__int8 FWFlags[10];
	__int8 TotGFlags[10];
	__int8 EarthFlags[18];
	__int8 WindFlags[10];

	WWFlags()
	{
		TowerRaised = 0;
		TingleFree = 0;
		TingleStatues = 0;
		GreatFairies = 0;
		memset(&DRCFlags, 0, sizeof(DRCFlags));
		memset(&FWFlags, 0, sizeof(FWFlags));
		memset(&TotGFlags, 0, sizeof(TotGFlags));
		memset(&EarthFlags, 0, sizeof(EarthFlags));
		memset(&WindFlags, 0, sizeof(WindFlags));
	}

	void GetFlagsFromProcess()
	{
		TowerRaised = GetFlag(TotGRaised);
		TingleFree = GetFlag(TingleIsFree);
		TingleStatues = GetFlag(DragonTingleStatue);
		GreatFairies = GetFlag(GreatFairyGift1);

		string currentStage = GetCurrentStage();

		// If player is in the dungeon, read from the local flags address instead of the permanent one
		if (wg_DRC.ContainsStage(currentStage))
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &DRCFlags, sizeof(DRCFlags), nullptr);
		else
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_DRC.permAddress), &DRCFlags, sizeof(DRCFlags), nullptr);

		if (wg_FW.ContainsStage(currentStage))
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &FWFlags, sizeof(FWFlags), nullptr);
		else
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_FW.permAddress), &FWFlags, sizeof(FWFlags), nullptr);

		if (wg_TotG.ContainsStage(currentStage))
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &TotGFlags, sizeof(TotGFlags), nullptr);
		else
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_TotG.permAddress), &TotGFlags, sizeof(TotGFlags), nullptr);

		if (wg_Earth.ContainsStage(currentStage))
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &EarthFlags, sizeof(EarthFlags), nullptr);
		else
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_Earth.permAddress), &EarthFlags, sizeof(EarthFlags), nullptr);

		if (wg_Wind.ContainsStage(currentStage))
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &WindFlags, sizeof(WindFlags), nullptr);
		else
			ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_Wind.permAddress), &WindFlags, sizeof(WindFlags), nullptr);
	}

	void StoreFlagsToProcess()
	{
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + TotGRaised.address), &TowerRaised, 1, nullptr);
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + TingleIsFree.address), &TingleFree, 1, nullptr);
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + DragonTingleStatue.address), &TingleStatues, 1, nullptr);
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + GreatFairyGift1.address), &GreatFairies, 1, nullptr);

		string currentStage = GetCurrentStage();

		// If player is in the dungeon, write to the local flags address instead of the permanent one
		if (wg_DRC.ContainsStage(currentStage))
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &DRCFlags, sizeof(DRCFlags), nullptr);
		else
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_DRC.permAddress), &DRCFlags, sizeof(DRCFlags), nullptr);

		if (wg_FW.ContainsStage(currentStage))
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &FWFlags, sizeof(FWFlags), nullptr);
		else
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_FW.permAddress), &FWFlags, sizeof(FWFlags), nullptr);

		if (wg_TotG.ContainsStage(currentStage))
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &TotGFlags, sizeof(TotGFlags), nullptr);
		else
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_TotG.permAddress), &TotGFlags, sizeof(TotGFlags), nullptr);

		if (wg_Earth.ContainsStage(currentStage))
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &EarthFlags, sizeof(EarthFlags), nullptr);
		else
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_Earth.permAddress), &EarthFlags, sizeof(EarthFlags), nullptr);
		
		if (wg_Wind.ContainsStage(currentStage))
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &WindFlags, sizeof(WindFlags), nullptr);
		else
			WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_Wind.permAddress), &WindFlags, sizeof(WindFlags), nullptr);
	}

	void PatchFlags(WWFlags newFlags) // Quick and dirty, will revise
	{
		char oldBuffer[sizeof(WWFlags)];
		char newBuffer[sizeof(WWFlags)];
		memcpy(&oldBuffer, this, sizeof(WWFlags));
		memcpy(&newBuffer, &newFlags, sizeof(WWFlags));

		for (int i = 0; i < sizeof(WWFlags); i++)
		{
			oldBuffer[i] = oldBuffer[i] | newBuffer[i];
		}

		memcpy(this, &oldBuffer, sizeof(WWFlags));
	}
};