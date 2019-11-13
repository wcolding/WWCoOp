#pragma once
#include "WWCore.h"

#define WWINV_BUFFER_LENGTH 128
#define WW_DEFAULT_PORT "2821"
#define WW_INTERVAL 1000

#define WW_COMMAND_POLL 0x0609
#define WW_COMMAND_SET	0x060A
#define WW_COMMAND_NAME 0x060B

#define WW_RESPONSE_POLL 0x0909
#define WW_RESPONSE_NAME 0x090B

bool verbose = true;

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