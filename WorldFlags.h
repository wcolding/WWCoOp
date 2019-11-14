#pragma once
#include "WWCore.h"
#include "DolphinHook.h"

#define WW_LOCAL_FLAGS 0x003C5382
#define WW_KEYS_OFFSET 0x1E

#define SCENE_COUNTER 0x003CA620

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

// Stage name is loaded before local flags are, so we need to let the game catch up before we attempt to sync
// This will effectively block the thread until we can ensure accurate local flags
void YieldToSceneCounter()
{
	int sceneCounter = 0;
	char counterBuffer[4];
	memset(&counterBuffer, 0, 4);

	while (sceneCounter == 0)
	{
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + SCENE_COUNTER), &counterBuffer, sizeof(counterBuffer), nullptr);
		for (int i = 0; i < 4; i++)
		{
			if (counterBuffer[i] != 0)
			{
				sceneCounter = 1;
				Sleep(1000);
			}
		}
		Sleep(1000/30); // Check once a frame (assuming 30 fps)
	}
}

// This will expand as we sync more flags
struct WWFlags
{
	__int8 TowerRaised;
	__int8 TingleFree;
	__int8 TingleStatues;
	__int8 GreatFairies;
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
};

int FlagsChanged(__int8 byte)
{
	int count = 0;

	if ((byte & 0x01) != 0)
		count++;
	if ((byte & 0x02) != 0)
		count++;
	if ((byte & 0x04) != 0)
		count++;
	if ((byte & 0x08) != 0)
		count++;
	if ((byte & 0x10) != 0)
		count++;
	if ((byte & 0x20) != 0)
		count++;
	if ((byte & 0x40) != 0)
		count++;
	if ((byte & 0x80) != 0)
		count++;

	return count;
}

int WWFlagsChanged(WWFlags oldFlags, WWFlags newFlags)
{
	int total = 0;
	int sceneCounter;
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + SCENE_COUNTER), &sceneCounter, sizeof(sceneCounter), nullptr);

	// Do not count flags if the game is loading a new room
	if (sceneCounter != 0)
	{
		total += FlagsChanged(oldFlags.TowerRaised ^ newFlags.TowerRaised);
		total += FlagsChanged(oldFlags.TingleFree ^ newFlags.TingleFree);
		total += FlagsChanged(oldFlags.TingleStatues ^ newFlags.TingleStatues);
		total += FlagsChanged(oldFlags.GreatFairies ^ newFlags.GreatFairies);

		for (int i = 0; i < sizeof(oldFlags.DRCFlags); i++)
			total += FlagsChanged(oldFlags.DRCFlags[i] ^ newFlags.DRCFlags[i]);
		for (int i = 0; i < sizeof(oldFlags.FWFlags); i++)
			total += FlagsChanged(oldFlags.FWFlags[i] ^ newFlags.FWFlags[i]);
		for (int i = 0; i < sizeof(oldFlags.TotGFlags); i++)
			total += FlagsChanged(oldFlags.TotGFlags[i] ^ newFlags.TotGFlags[i]);
		for (int i = 0; i < sizeof(oldFlags.EarthFlags); i++)
			total += FlagsChanged(oldFlags.EarthFlags[i] ^ newFlags.EarthFlags[i]);
		for (int i = 0; i < sizeof(oldFlags.WindFlags); i++)
			total += FlagsChanged(oldFlags.WindFlags[i] ^ newFlags.WindFlags[i]);
	}

	return total;
}

WWFlags GetFlagsFromProcess()
{
	WWFlags flags;
	flags.TowerRaised = GetFlag(TotGRaised);
	flags.TingleFree = GetFlag(TingleIsFree);
	flags.TingleStatues = GetFlag(DragonTingleStatue);
	flags.GreatFairies = GetFlag(GreatFairyGift1);

	string currentStage = GetCurrentStage();

	// If player is in the dungeon, read from the local flags address instead of the permanent one
	if (wg_DRC.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.DRCFlags, sizeof(flags.DRCFlags), nullptr);
	}
	else
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_DRC.permAddress), &flags.DRCFlags, sizeof(flags.DRCFlags), nullptr);

	if (wg_FW.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.FWFlags, sizeof(flags.FWFlags), nullptr);
	}
	else
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_FW.permAddress), &flags.FWFlags, sizeof(flags.FWFlags), nullptr);

	if (wg_TotG.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.TotGFlags, sizeof(flags.TotGFlags), nullptr);
	}
	else
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_TotG.permAddress), &flags.TotGFlags, sizeof(flags.TotGFlags), nullptr);

	if (wg_Earth.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.EarthFlags, sizeof(flags.EarthFlags), nullptr);
	}
	else
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_Earth.permAddress), &flags.EarthFlags, sizeof(flags.EarthFlags), nullptr);

	if (wg_Wind.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.WindFlags, sizeof(flags.WindFlags), nullptr);
	}
	else
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_Wind.permAddress), &flags.WindFlags, sizeof(flags.WindFlags), nullptr);
	
	return flags;
}

void StoreFlagsToProcess(WWFlags flags)
{
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + TotGRaised.address), &flags.TowerRaised, 1, nullptr);
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + TingleIsFree.address), &flags.TingleFree, 1, nullptr);
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + DragonTingleStatue.address), &flags.TingleStatues, 1, nullptr);
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + GreatFairyGift1.address), &flags.GreatFairies, 1, nullptr);

	string currentStage = GetCurrentStage();

	// If player is in the dungeon, write to the local flags address instead of the permanent one
	if (wg_DRC.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.DRCFlags, sizeof(flags.DRCFlags), nullptr);
	}
	else
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_DRC.permAddress), &flags.DRCFlags, sizeof(flags.DRCFlags), nullptr);

	if (wg_FW.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.FWFlags, sizeof(flags.FWFlags), nullptr);
	}
	else
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_FW.permAddress), &flags.FWFlags, sizeof(flags.FWFlags), nullptr);

	if (wg_TotG.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.TotGFlags, sizeof(flags.TotGFlags), nullptr);
	}
	else
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_TotG.permAddress), &flags.TotGFlags, sizeof(flags.TotGFlags), nullptr);

	if (wg_Earth.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.EarthFlags, sizeof(flags.EarthFlags), nullptr);
	}
	else
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_Earth.permAddress), &flags.EarthFlags, sizeof(flags.EarthFlags), nullptr);

	if (wg_Wind.ContainsStage(currentStage))
	{
		YieldToSceneCounter();
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags.WindFlags, sizeof(flags.WindFlags), nullptr);
	}
	else
		WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg_Wind.permAddress), &flags.WindFlags, sizeof(flags.WindFlags), nullptr);
}

void PatchFlags(WWFlags oldFlags, WWFlags newFlags) // May not even use this
{
	if (WWFlagsChanged(oldFlags, newFlags) > 0)
	{
		char oldBuffer[sizeof(WWFlags)];
		char newBuffer[sizeof(WWFlags)];
		memcpy(&oldBuffer, &oldFlags, sizeof(WWFlags));
		memcpy(&newBuffer, &newFlags, sizeof(WWFlags));

		for (int i = 0; i < sizeof(WWFlags); i++)
		{
			oldBuffer[i] = oldBuffer[i] | newBuffer[i];
		}

		memcpy(&oldFlags, &oldBuffer, sizeof(WWFlags));
	}
}