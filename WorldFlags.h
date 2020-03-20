#pragma once
#include "WWCore.h"
#include "DolphinHook.h"

#define WW_LOCAL_FLAGS 0x003C5380
#define WW_FLAGS_SIZE 12
#define WW_KEYS_OFFSET 0x20

#define SCENE_COUNTER 0x003CA620

struct WorldFlag
{
	unsigned int address;
	__int8 flag;
};

// Describes a set of stages with shared flags, such as for islands or dungeons
struct WorldGroup
{
	unsigned int permAddress;
	vector <string> stageNames;
	__int8 flags[WW_FLAGS_SIZE];

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

void GetWorldGroupFlags(WorldGroup* wg)
{
	ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg->permAddress), &wg->flags, WW_FLAGS_SIZE, nullptr);
}

void SetWorldGroupFlags(WorldGroup* wg)
{
	WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + wg->permAddress), &wg->flags, WW_FLAGS_SIZE, nullptr);
}

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

WorldGroup wg_OutsetInterior = { 0x003C5116, {"LinkUG", "Omasao", "A_mori"} }; // This address gets reused as the last non-permanent flags in readable memory? Maybe? idk yet


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
	__int8 GreatFairies;
	WorldGroup wg_DRC   = { 0x003C4FF4, {"M_NewD2", "M_Dra09", "M_DragB"} };
	WorldGroup wg_FW    = { 0x003C5018, {"kindan", "kinMB", "kinBOSS"} };
	WorldGroup wg_TotG  = { 0x003C503C, {"Siren", "SirenMB", "SirenB"} };
	WorldGroup wg_Earth = { 0x003C5060, {"M_Dai", "M_DaiMB", "M_DaiB"} };
	WorldGroup wg_Wind  = { 0x003C5084, {"kaze", "kazeMB", "kazeB"} };
	__int8 DRC_keys;
	__int8 FW_keys;
	__int8 TotG_keys;
	__int8 Earth_keys;
	__int8 Wind_keys;

	WWFlags()
	{
		TowerRaised = 0;
		TingleFree = 0;
		GreatFairies = 0;
		memset(&wg_DRC.flags, 0, WW_FLAGS_SIZE);
		memset(&wg_FW.flags, 0, WW_FLAGS_SIZE);
		memset(&wg_TotG.flags, 0, WW_FLAGS_SIZE);
		memset(&wg_Earth.flags, 0, WW_FLAGS_SIZE);
		memset(&wg_Wind.flags, 0, WW_FLAGS_SIZE);
		DRC_keys = 0;
		FW_keys = 0;
		TotG_keys = 0;
		Earth_keys = 0;
		Wind_keys = 0;
	}

	// To be called when the player is NOT in a dungeon
	// Fetches all dungeon flags and keys from Dolphin
	void GetDungeons()
	{
		// Dragon Roost Cavern
		GetWorldGroupFlags(&wg_DRC);
		DRC_keys = DolphinRead8(WW_KEYS_OFFSET + wg_DRC.permAddress);

		// Forbidden Woods
		GetWorldGroupFlags(&wg_FW);
		FW_keys = DolphinRead8(WW_KEYS_OFFSET + wg_FW.permAddress);

		// Tower of the Gods
		GetWorldGroupFlags(&wg_TotG);
		TotG_keys = DolphinRead8(WW_KEYS_OFFSET + wg_TotG.permAddress);

		// Earth Temple
		GetWorldGroupFlags(&wg_Earth);
		Earth_keys = DolphinRead8(WW_KEYS_OFFSET + wg_Earth.permAddress);

		// Wind Temple
		GetWorldGroupFlags(&wg_Wind);
		Wind_keys = DolphinRead8(WW_KEYS_OFFSET + wg_Wind.permAddress);
	}

	// To be called when the player is NOT in a dungeon
	// Writes a single dungeon's flags and keys to Dolphin
	void SetDungeon(WorldGroup* dungeon)
	{
		SetWorldGroupFlags(dungeon);

		if (dungeon == &wg_DRC)
			DolphinWrite8(WW_KEYS_OFFSET + wg_DRC.permAddress, DRC_keys);
		if (dungeon == &wg_FW)
			DolphinWrite8(WW_KEYS_OFFSET + wg_FW.permAddress, FW_keys);
		if (dungeon == &wg_TotG)
			DolphinWrite8(WW_KEYS_OFFSET + wg_TotG.permAddress, TotG_keys);
		if (dungeon == &wg_Earth)
			DolphinWrite8(WW_KEYS_OFFSET + wg_Earth.permAddress, Earth_keys);
		if (dungeon == &wg_Wind)
			DolphinWrite8(WW_KEYS_OFFSET + wg_Wind.permAddress, Wind_keys);

	}
};

// Merges the second set of flags with the first's set of flags and stores it to the first set
void MergeFlags(__int8 (&a)[WW_FLAGS_SIZE], __int8(&b)[WW_FLAGS_SIZE])
{
	for (int i = 0; i < WW_FLAGS_SIZE; i++)
		a[i] |= b[i];
}

struct LocalContext
{
	__int8 flags[18];
	string stageName;
	int sceneCounter;

	LocalContext()
	{
		memset(&flags, 0, sizeof(flags));
		stageName = "";
		sceneCounter = 0;
	}

	void Update()
	{
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WW_LOCAL_FLAGS), &flags, sizeof(flags), nullptr);
		stageName = GetCurrentStage();
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + SCENE_COUNTER), &sceneCounter, 4, nullptr);
	}

	char* Serialize()
	{
		char buffer[32];
		memset(&buffer, 0, sizeof(buffer));
		memcpy(&buffer, &flags, sizeof(flags));
		memcpy(&buffer[sizeof(flags)], stageName.c_str(), 9); // limit to 10 characters, last being null
		memcpy(&buffer[sizeof(flags) + 10], &sceneCounter, sizeof(sceneCounter));
		return buffer;
	}
};

