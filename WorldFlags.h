#pragma once
#include "WWCore.h"
#include "DolphinHook.h"

#define WW_STAGE_INFO_LEN 0x24 // I'm counting 0x22?
#define WW_LOCAL_FLAGS 0x003C5380
#define WW_FLAGS_SIZE 12
#define WW_DUNG_KEYS 0x20
#define WW_DUNG_PROGRESS 0x21
#define WW_DUNG_VISTED 0x18

#define STAGE_ID 0x003C53A4
#define SCENE_COUNTER 0x003CA620

struct StageInfo
{
	__int8 chestFlags[4];
	__int8 eventSwitches[16];
	__int8 itemPickupFlags[4];
	__int8 visitedRooms[4];
	__int8 unknown[4]; // possibly padding?
	__int8 smallKeys;
	__int8 dungeonProgress;
};

struct Stage
{
	__int8 stageID;
	DWORD address;
	vector <string> sceneNames;
	StageInfo info;

	bool ContainsScene(string scene)
	{
		size_t numScenes = sceneNames.size();
		
		if (numScenes < 1)
			return false;

		for (int i = 0; i < numScenes; i++)
		{
			if (sceneNames[i] == scene)
				return true;
		}
		return false;
	}

	// Reads current flags from Dolphin
	void UpdateInfo()
	{
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + address), &info, sizeof(StageInfo), nullptr);
	}
};

// For anything not strictly stored within StageInfo
struct WorldFlag
{
	DWORD address;
	__int8 flag;
};

// Collection of WorldFlags to be shared over the network
struct WorldFlagGroup
{
	vector<WorldFlag> worldFlags =
	{
		{ 0x003C5240, 0 },	// Pearls Placed on Triangle Isles
		{ 0x003C524A, 0 },  // Tower of the Gods Raised
		{ 0x003C5237, 0 },  // Tingle is freed from jail
		{ 0x003C525C, 0 }   // Great Fairies visited
	};

	void ReadFlags()
	{
		for (int i = 0; i < worldFlags.size(); i++)
			worldFlags[i].flag = DolphinRead8(worldFlags[i].address);
	}

	void WriteFlags()
	{
		for (int i = 0; i < worldFlags.size(); i++)
			DolphinWrite8(worldFlags[i].address, worldFlags[i].flag);
	}
};

// Used to cache local player's StageInfo data
struct WWFlags
{
	Stage Stages[15] =
	{
		{ 0x00, 0x003C4F88, {"ADMumi", "ENDumi", "sea"} },       // Sea
		{ 0x01, 0x003C4FAC },                                    // Sea Additional
		{ 0x02, 0x003C4FD0, {"M2ganon", "M2tower", "MajyuE",
		                     "Mjtower", "ma2room", "ma3room", 
							 "majroom"} },                       // Forsaken Fortress
		{ 0x03, 0x003C4FF4, {"M_Dra09", "M_DragB", "M_NewD2"} }, // Dragon Roost Cavern
		{ 0x04, 0x003C5018, {"kinBOSS", "kinMB", "kindan"} },    // Forbidden Woods
		{ 0x05, 0x003C503C, {"Siren", "SirenB", "SirenMB"} },    // Tower of the Gods
		{ 0x06, 0x003C5060, {"M_Dai", "M_DaiB", "M_DaiMB"} },    // Earth Temple
		{ 0x07, 0x003C5084, {"kaze", "kazeB", "kazeMB"} },       // Wind Temple
		{ 0x08, 0x003C50A8, {"GTower", "GanonA", "GanonB", 
		                     "GanonC", "GanonD", "GanonE", 
							 "GanonJ", "GanonK", "GanonL", 
							 "GanonM", "GanonN", "Xboss0", 
							 "Xboss1", "Xboss2", "Xboss3"} },    // Ganon's Tower
		{ 0x09, 0x003C50CC, {"Hyroom", "Hyrule", "kenroom"} },   // Hyrule
		{ 0x0A, 0x003C50F0, {"A_umikz", "Abship", "Obshop", 
		                     "PShip"} },                         // Ships
		{ 0x0B, 0x003C5114, {"A_mori", "Abesso", "Adanmae", 
		                     "Atorizk", "Comori", "Edaichi", 
							 "Ekaze", "Kaisen", "LinkRM", 
							 "LinkUG", "Nitiyou", "Obombh", 
							 "Ocmera", "Ocrogh", "Ojhous2", 
							 "Ojhous", "Omasao", "Omori", 
							 "Onobuta", "Opub", "Orichh", 
							 "Otkura", "Pdrgsh", "Pfigure", 
							 "Pjavdou", "Pnezumi", "figureA", 
							 "figureB", "figureC", "figureD", 
							 "figureE", "figureF", "figureG"} }, // Town/Misc
		{ 0x0C, 0x003C5138, {"Cave01", "Cave02", "Cave03", 
		                     "Cave04", "Fairy01", "Fairy02", 
							 "Fairy03", "Fairy04", "Fairy05", 
							 "Fairy06", "ITest62", "MiniHyo", 
							 "MiniKaz", "PShip2", "PShip3", 
							 "ShipD", "TF_01", "TF_02", 
							 "TF_03", "TF_04", "TyuTyu", 
							 "WarpD"} },                         // Caves
		{ 0x0D, 0x003C515C, {"Asoko", "Cave05", "Cave07", 
		                     "Cave09", "Cave10", "Cave11", 
							 "ITest63", "SubD42", "SubD43", 
							 "SubD71", "TF_06"} },               // Caves + Ships
		{ 0x0E, 0x003C5180 }                                     // ChuChu flags
	};
};


// Takes a WWFlags instance by reference and reads flag values to it from Dolphin
void UpdateWWFlags(WWFlags *flags)
{
	for (int i = 0; i < 15; i++)
		flags->Stages[i].UpdateInfo();
}

// ORs two sets of StageInfo and returns a third combined set
StageInfo MergeStageInfo(StageInfo a, StageInfo b)
{
	StageInfo newInfo;
	const size_t infoSize = sizeof(StageInfo);
	char _a[infoSize];
	char _b[infoSize];
	memcpy(&_a, &a, infoSize);
	memcpy(&_b, &b, infoSize);
	for (int i = 0; i < infoSize; i++)
		_a[i] |= _b[i];
	memcpy(&newInfo, &_a, infoSize);
	return newInfo;
}

typedef enum : __int8
{
	Map       = 0x01,
	Compass   = 0x02,
	BigKey    = 0x04,
	BossDead  = 0x08,
	Heart     = 0x10,
	BossIntro = 0x20
} WWDungeonMask;

typedef enum : __int8
{
	PlacedNayru  = 0x10,
	PlacedFarore = 0x40,
	PlacedDin    = 0x80
} WWPearlsPlacedMask;

typedef enum : __int8
{
	Thorned  = 0x01,
	Western  = 0x02,
	Southern = 0x04,
	Eastern  = 0x08,
	Outset   = 0x10,
	Northern = 0x20,
} WWGreatFairyMask;

// Describes a set of stages with shared flags, such as for islands or dungeons
struct WorldGroup
{
	unsigned int permAddress;
	__int8 stageID;
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

// Stage name is loaded before local flags are, so we need to let the game catch up before we attempt to sync
// This will effectively block the thread until we can ensure accurate local flags
void YieldToSceneCounter()
{
	short sceneCounter = 0;

	while (sceneCounter == 0)
	{
		ReadProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + SCENE_COUNTER -2), &sceneCounter, 2, nullptr);
		Sleep(1000/30); // Check once a frame (assuming 30 fps)
	}
}

// This will expand as we sync more flags
//struct WWFlags
//{
//	
//	__int8 TowerRaised = 0;
//	__int8 TingleFree = 0;
//	__int8 GreatFairies = 0;
//
//	Dungeon d_DRC, d_FW, d_TotG, d_Earth, d_Wind;
//
//	WWFlags()
//	{
//		d_DRC.wg = { 0x003C4FF4, 0x03, {"M_NewD2", "M_Dra09", "M_DragB"} };
//		d_FW.wg = { 0x003C5018, 0x04, {"kindan", "kinMB", "kinBOSS"} };
//		d_TotG.wg = { 0x003C503C, 0x05, {"Siren", "SirenMB", "SirenB"} };
//		d_Earth.wg = { 0x003C5060, 0x06, {"M_Dai", "M_DaiMB", "M_DaiB"} };
//		d_Wind.wg = { 0x003C5084, 0x07, {"kaze", "kazeMB", "kazeB"} };
//		memset(&d_DRC.wg.flags, 0, WW_FLAGS_SIZE);
//		memset(&d_FW.wg.flags, 0, WW_FLAGS_SIZE);
//		memset(&d_TotG.wg.flags, 0, WW_FLAGS_SIZE);
//		memset(&d_Earth.wg.flags, 0, WW_FLAGS_SIZE);
//		memset(&d_Wind.wg.flags, 0, WW_FLAGS_SIZE);
//	}
//
//	// To be called when the player is NOT in a dungeon
//	// Fetches all dungeon flags and keys from Dolphin
//	void GetDungeons()
//	{
//		// Dragon Roost Cavern
//		GetWorldGroupFlags(&d_DRC.wg);
//		d_DRC.keys = DolphinRead8(WW_DUNG_KEYS + d_DRC.wg.permAddress);
//		d_DRC.progress = DolphinRead8(WW_DUNG_PROGRESS + d_DRC.wg.permAddress);
//		d_DRC.visited = DolphinRead16(WW_DUNG_VISTED + d_DRC.wg.permAddress);
//
//		// Forbidden Woods
//		GetWorldGroupFlags(&d_FW.wg);
//		d_FW.keys = DolphinRead8(WW_DUNG_KEYS + d_FW.wg.permAddress);
//		d_FW.progress = DolphinRead8(WW_DUNG_PROGRESS + d_FW.wg.permAddress);
//		d_FW.visited = DolphinRead16(WW_DUNG_VISTED + d_FW.wg.permAddress);
//
//		// Tower of the Gods
//		GetWorldGroupFlags(&d_TotG.wg);
//		d_TotG.keys = DolphinRead8(WW_DUNG_KEYS + d_TotG.wg.permAddress);
//		d_TotG.progress = DolphinRead8(WW_DUNG_PROGRESS + d_TotG.wg.permAddress);
//		d_TotG.visited = DolphinRead16(WW_DUNG_VISTED + d_TotG.wg.permAddress);
//
//		// Earth Temple
//		GetWorldGroupFlags(&d_Earth.wg);
//		d_Earth.keys = DolphinRead8(WW_DUNG_KEYS + d_Earth.wg.permAddress);
//		d_Earth.progress = DolphinRead8(WW_DUNG_PROGRESS + d_Earth.wg.permAddress);
//		d_Earth.visited = DolphinRead16(WW_DUNG_VISTED + d_Earth.wg.permAddress);
//
//		// Wind Temple
//		GetWorldGroupFlags(&d_Wind.wg);
//		d_Wind.keys = DolphinRead8(WW_DUNG_KEYS + d_Wind.wg.permAddress);
//		d_Wind.progress = DolphinRead8(WW_DUNG_PROGRESS + d_Wind.wg.permAddress);
//		d_Wind.visited = DolphinRead16(WW_DUNG_VISTED + d_Wind.wg.permAddress);
//	}
//
//	// To be called when the player is NOT in a dungeon
//	// Writes a single dungeon's progress to Dolphin
//	void SetDungeon(Dungeon d)
//	{
//		SetWorldGroupFlags(&d.wg);
//		DolphinWrite8(WW_DUNG_KEYS + d.wg.permAddress, d.keys);
//		DolphinWrite8(WW_DUNG_PROGRESS + d.wg.permAddress, d.progress);
//		DolphinWrite16(WW_DUNG_VISTED + d.wg.permAddress, d.visited);
//	}
//};

//// Merges the second set of flags with the first's set of flags and stores it to the first set
//void MergeFlags(__int8 (&a)[WW_FLAGS_SIZE], __int8(&b)[WW_FLAGS_SIZE])
//{
//	for (int i = 0; i < WW_FLAGS_SIZE; i++)
//		a[i] |= b[i];
//}

