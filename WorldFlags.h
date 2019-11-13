#pragma once
#include "WWCore.h"
#include "DolphinHook.h"

#define WW_TEMP_FLAGS 0x003C5382

struct WorldFlag
{
	unsigned int address;
	__int8 flag;
};

WorldFlag DinsPearlPlaced = { 0x003C5240, 0b10000000 };
WorldFlag FaroresPearlPlaced = { 0x003C5240, 0b01000000 };
WorldFlag NayrusPearlPlaced = { 0x003C5240, 0b00010000 };

WorldFlag TotGRaised = { 0x003C524A, 0b01000000 };

WorldFlag TingleFree			= { 0x003C5237, 0b10000000 }; // Cannot redeem statue rewards if this isn't set
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
};

WorldGroup OutsetInterior = { 0x003C5116, {"LinkUG", "Omasao", "A_mori"} };

struct WorldGroupFlag
{
	WorldGroup group;
	unsigned int offset;
	__int8 flag;
};

// We probably won't individually address these but it's still useful information
WorldGroupFlag OutsetLinksHouseChest = { OutsetInterior, 1, 0b00100000 };
WorldGroupFlag OutsetGrasscutterChest = { OutsetInterior, 1, 0b00010000 };
WorldGroupFlag OutsetFairyBoulder = { OutsetInterior, 4, 0b00010000 };

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
