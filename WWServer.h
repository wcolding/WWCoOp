#pragma once
#include "WWCore.h"
#include "Inventory.h"

#define WWINV_BUFFER_LENGTH 256
#define WW_DEFAULT_PORT "2821"
#define WW_INTERVAL 1000

#define WW_COMMAND_POLL 0x0609
#define WW_COMMAND_SET	0x060A
#define WW_COMMAND_NAME 0x060B
#define WW_COMMAND_SET_FLAG 0x060C

#define WW_RESPONSE_POLL 0x0909
#define WW_RESPONSE_NAME 0x090B
#define WW_RESPONSE_FLAG 0x090C

bool verbose = false;

struct Player
{
	char name[16];
	int checksumA;
	int checksumB;
	WWInventory inventory;
	// Flags here

};

// Clears the given buffer and writes a command to the header
void SetBufferCommand(char (&buffer)[WWINV_BUFFER_LENGTH], short command)
{
	memset(&buffer, 0, sizeof(buffer));
	char commandBuffer[2];
	memcpy(&commandBuffer, &command, 2);
	// TO DO: endian test
	buffer[0] = commandBuffer[1];
	buffer[1] = commandBuffer[0];
}

// Returns command from a given buffer
short GetBufferCommand(char (&buffer)[WWINV_BUFFER_LENGTH])
{
	char commandBuffer[2];
	char swapBuffer[2];
	memcpy(&commandBuffer, &buffer, 2);
	// TO DO: endian test
	swapBuffer[0] = commandBuffer[1];
	swapBuffer[1] = commandBuffer[0];

	short command = 0;
	memcpy(&command, &swapBuffer, 2);
	return command;
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

	// Third byte
	// XOR chart masks A and B, then XOR each byte down to an 8 bit value
	unsigned int chartMaskC = inv.Charts.a ^ inv.Charts.b;
	char chartsBuffer[4];
	memcpy(&chartsBuffer, &chartMaskC, 4);
	for (int i = 0; i < 4; i++)
		checksum[2] ^= chartsBuffer[i];

	// Fourth byte
	// Add the other values together, then XOR the buttons
	int sum = inv.Wallet + inv.Magic + inv.Quiver + inv.BombBag + inv.Hearts + inv.PiecesofHeart;
	sum ^= inv.XButtonEquip;
	sum ^= inv.YButtonEquip;
	sum ^= inv.ZButtonEquip;
	checksum[3] = sum >> 24;

	int c = 0;
	memcpy(&c, &checksum, 4);
	return c;
}