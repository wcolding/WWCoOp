#pragma once
#include "WWCore.h"
#include "Inventory.h"
#include "WorldFlags.h"

#define WWINV_BUFFER_LENGTH 256
#define WW_DEFAULT_PORT "2821"
#define WW_INTERVAL 1000

#define WW_COMMAND_POLL 0x0609
#define WW_COMMAND_SET	0x060A   // WW_COMMAND_SET, address, value, length
#define WW_COMMAND_NAME 0x060B
#define WW_COMMAND_SET_CHARTS 0x060C // WW_COMMAND_SET_CHARTS, value
#define WW_COMMAND_GIVE_ITEM 0x060D

#define WW_RESPONSE_POLL 0x0909
#define WW_RESPONSE_NAME 0x090B
#define WW_RESPONSE_FLAG 0x090C

bool verbose = false;

struct Player
{
	char name[16]; // we will limit this to 15 characters plus a null byte
	int checksumA = 0;
	int checksumB = 0;
	WWInventory inventory;
	WWFlags flags;

	void SetName(string n)
	{
		memset(&name, 0, sizeof(name));
		memcpy(&name, n.c_str(), 15);
	}
};

// Clears the given buffer and writes a command to the header
void SetBufferCommand(char (&buffer)[WWINV_BUFFER_LENGTH], short command)
{
	memset(&buffer, 0, sizeof(buffer));
	char commandBuffer[2];
	memcpy(&commandBuffer, &command, 2);
	buffer[0] = commandBuffer[1];
	buffer[1] = commandBuffer[0];
}

// Returns command from a given buffer
short GetBufferCommand(char (&buffer)[WWINV_BUFFER_LENGTH])
{
	char commandBuffer[2];
	char swapBuffer[2];
	memcpy(&commandBuffer, &buffer, 2);
	swapBuffer[0] = commandBuffer[1];
	swapBuffer[1] = commandBuffer[0];

	short command = 0;
	memcpy(&command, &swapBuffer, 2);
	return command;
}

// Automates the sending of a WW_COMMAND_SET packet to a client
// this is buggy as hell and probably a bad idea the more I think about it
int ClientSetValue(SOCKET client, unsigned int _address, char *data, size_t dataLen )
{
	char buffer[WWINV_BUFFER_LENGTH];
	SetBufferCommand(buffer, WW_COMMAND_SET);
	unsigned int address = BASE_OFFSET + _address;
	memcpy(&buffer[2], &address, 4); // address
	memcpy(&buffer[6], &data, dataLen); // value
	memcpy(&buffer[6+dataLen], &dataLen, 4); // size to write

	return send(client, buffer, 6 + dataLen + 4, 0);
}

int ClientGiveItem(SOCKET client, WWItem item)
{
	char buffer[WWINV_BUFFER_LENGTH];
	SetBufferCommand(buffer, WW_COMMAND_GIVE_ITEM);
	buffer[2] = item;
	return send(client, buffer, 3, 0);
}

// Chooses whether to update a local or remote player's itemState at a specified index
void TestItemStates(SOCKET client, Player localPlayer, Player remotePlayer, int index)
{
	if (index >= sizeof(localPlayer.inventory.itemStates))
		return;

	if (localPlayer.inventory.itemStates[index] != remotePlayer.inventory.itemStates[index])
	{
		bool setClient = (localPlayer.inventory.itemStates[index] > remotePlayer.inventory.itemStates[index]);
		if (!setClient)
		{
			// Update local player
			localPlayer.inventory.itemStates[index] = remotePlayer.inventory.itemStates[index];
			DolphinWrite8(InventoryMap[index].address, InventoryMap[index].states[localPlayer.inventory.itemStates[index]].item);
		}
		else
		{
			// Update remote player
			//ClientSetValue(client, InventoryMap[index].address, &InventoryMap[index].states[localPlayer.inventory.itemStates[index]].item, 1);
			ClientGiveItem(client, (WWItem)InventoryMap[index].states[localPlayer.inventory.itemStates[index]].item);
		}
	}
}

// Prints a message if verbose mode is enabled
void LogVerbose(const char* msg)
{
	if (verbose)
		std::cout << msg << std::endl;
}

void LogVerbose(const char* msg, int val)
{
	if (verbose)
		std::cout << val << msg << std::endl;
}

int CalculateChecksum(WWInventory inv)
{
	char checksum[4];
	memset(&checksum, 0, 4);

	// First byte
	// XOR all items as defined by itemStates and InventoryMap
	for (int i = 0; i < sizeof(inv.itemStates); i++)
	{
		checksum[0] ^= InventoryMap[i].states[inv.itemStates[i]].item;
	}

	// Second byte
	// XOR bitmasked values, with songs inverted
	checksum[1] ^= (char)(0xFF - inv.Songs);
	checksum[1] ^= inv.Triforce;
	checksum[1] ^= inv.Pearls;
	checksum[1] ^= inv.Statues;

	// Third byte
	// XOR chart masks A and B, then XOR each byte down to an 8 bit value
	unsigned int chartMaskC = inv.Charts.a ^ inv.Charts.b;
	char chartsBuffer[4];
	memcpy(&chartsBuffer, &chartMaskC, 4);
	for (int i = 0; i < 4; i++)
		checksum[2] ^= chartsBuffer[i];

	// Fourth byte
	// Add the other values together, dump the sum into an array, XOR the array
	int sum = inv.Wallet + inv.Magic + inv.Quiver + inv.BombBag + inv.Hearts + inv.PiecesofHeart;
	char sumBuffer[4];
	memcpy(&sumBuffer, &sum, sizeof(sum));
	checksum[3] = sumBuffer[0];
	checksum[3] ^= sumBuffer[1];
	checksum[3] ^= sumBuffer[2];
	checksum[3] ^= sumBuffer[3];

	int c = 0;
	memcpy(&c, &checksum, 4);
	return c;
}

int CalculateChecksum(WWFlags flags)
{
	int checksum = 0;
	char* serialized = (char*)malloc(sizeof(flags));
	memcpy(&serialized, &flags, sizeof(flags));

	for (int i = 0; i < sizeof(serialized); i++)
	{
		if (i % 2 == 0)
		{
			checksum += serialized[i];
		}
		else
		{
			checksum ^= serialized[i];
		}
	}
	free(serialized);

	return checksum;
}