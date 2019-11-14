#include "WWCore.h"
#include "DolphinHook.h"
#include "WWServer.h"
#include "WorldFlags.h"

void ShowUsage();
UINT ListenThread(LPVOID listener);
UINT NewClientThread(LPVOID newClient);

bool running = true;

WWInventory localUserInv, swapInv, patchInv;
WWFlags localUserFlags, swapFlags;

vector <string> clientNames;

int main(int argc, char *argv[])
{
	std::cout << "Wind Waker Co-op " << VERSION << std::endl << std::endl;

	struct addrinfo* result = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	WSADATA wsa;
	int iResult;
	u_long blockingFlags = 1;
	timeval timeout;
	timeout.tv_sec = 1;

	char buffer[WWINV_BUFFER_LENGTH];
	char sendBuffer[WWINV_BUFFER_LENGTH]; 
	memset(&buffer, 0, sizeof(buffer));
	memset(&sendBuffer, 0, sizeof(sendBuffer));
	int bytesRead = 0;

	vector<string> itemsList;
	
	// Server configuration
	if (argv[1] == string("-s") || argv[1] == string("-S"))
	{
		if (argc != 3)
		{
			ShowUsage();
			return -1;
		}

		SOCKET listener;

		iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (iResult != 0)
			return -2; // WSAStartup failed

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		iResult = getaddrinfo(NULL, argv[2], &hints, &result);
		if (iResult != 0)
		{
			int error = WSAGetLastError();
			std::cout << error << std::endl;
			WSACleanup();
			return -3;
		}

		listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (listener == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			int error = WSAGetLastError();
			std::cout << error << std::endl;
			WSACleanup();
			return -4;
		}

		iResult = bind(listener, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(listener);
			int error = WSAGetLastError();
			std::cout << error << std::endl;
			WSACleanup();
			return -5;
		}
		
		HookDolphinProcess();
		if (DolphinHandle == NULL)
		{
			std::cout << "Unable to hook Dolphin process." << std::endl;
			closesocket(listener);
			WSACleanup();
			return -6;
		}

		listen(listener, SOMAXCONN);
		AfxBeginThread(ListenThread, (LPVOID)listener);

		std::cout << "Server started on port " << argv[2] << "..." << std::endl << std::endl;

		localUserInv = GetInventoryFromProcess();
		PrintInventory(localUserInv);

		while (running)
		{
			swapInv = GetInventoryFromProcess();
			swapFlags = GetFlagsFromProcess();

			if (InvChanged(localUserInv, swapInv))
			{
				patchInv = MakePatch(localUserInv, swapInv);
				PrintInventory(patchInv);
				localUserInv = swapInv;
			}

			if (WWFlagsChanged(localUserFlags, swapFlags) > 0)
			{
				PatchFlags(localUserFlags, swapFlags);
			}
			Sleep(WW_INTERVAL);
		}

		CloseHandle(DolphinHandle);
		return 0;
	}

	// Client configuration
	else if (argv[1] == string("-c") || argv[1] == string("-C"))
	{
		if (argc != 4)
		{
			ShowUsage();
			return -1;
		}

		struct addrinfo* ptr = NULL;
		SOCKET client = NULL;

		iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (iResult != 0)
			return -2; // WSAStartup failed

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		iResult = getaddrinfo(argv[2], argv[3], &hints, &result);
		if (iResult != 0)
		{
			int error = WSAGetLastError();
			std::cout << error << std::endl;
			WSACleanup();
			return -3;
		}

		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			client = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (client == INVALID_SOCKET)
			{
				freeaddrinfo(result);
				int error = WSAGetLastError();
				std::cout << error << std::endl;
				WSACleanup();
				return -4;
			}

			iResult = connect(client, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				closesocket(client);
				client = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(result);

		if (client == INVALID_SOCKET)
		{
			WSACleanup();
			return -5;
		}

		std::cout << "Connected to server at " << argv[2] << ":" << argv[3] << std::endl;

		HookDolphinProcess();
		if (DolphinHandle == NULL)
		{
			std::cout << "Unable to hook Dolphin process." << std::endl;
			return -6;
		}

		/* Listen to first 2 bytes:
		 * 0x0609 - Request for inventory/flags
		 * 0x060A -	Write this incoming patch to the game */

		WWInventory rxPatch;
		WWFlags rxFlags;

		localUserInv = GetInventoryFromProcess();
		PrintInventory(localUserInv);

		while (running)
		{
			memset(&buffer, 0, sizeof(buffer));
			memset(&sendBuffer, 0, sizeof(sendBuffer));
			bytesRead = recv(client, buffer, sizeof(buffer), 0);

			if (bytesRead >= 2)
			{
				LogVerbose("Message received from server");
				short command = 0;
				command = GetBufferCommand(buffer);

				switch (command)
				{
				case WW_COMMAND_POLL:
				{
					localUserInv = GetInventoryFromProcess();
					localUserFlags = GetFlagsFromProcess();

					// Lazy serialization
					SetBufferCommand(sendBuffer, WW_RESPONSE_POLL);
					memcpy(&sendBuffer[2], &localUserInv, sizeof(WWInventory));
					memcpy(&sendBuffer[2 + sizeof(WWInventory)], &localUserFlags, sizeof(WWFlags));
					send(client, sendBuffer, 2 + sizeof(WWInventory) + sizeof(WWFlags), 0);
					LogVerbose("Inventory sent to server");
					break;
				}
				case WW_COMMAND_SET:
				{
					// Lazy deserialization
					memcpy(&rxPatch, &buffer[2], sizeof(WWInventory));
					memcpy(&rxFlags, &buffer[2 + sizeof(WWInventory)], sizeof(WWFlags));

					LogVerbose("Inventory items received from server");

					PrintInventory(rxPatch);
					StoreInventoryToProcess(rxPatch);

					localUserFlags = GetFlagsFromProcess();

					if (WWFlagsChanged(localUserFlags, rxFlags) > 0)
					{
						PatchFlags(localUserFlags, rxFlags);
						StoreFlagsToProcess(localUserFlags);
					}

					break;
				}
				default:
					break;
				}
			}

		}

		CloseHandle(DolphinHandle);
		return 0;
	}
	// Test mode, no networking
	else if (argv[1] == string("-testmode"))
	{
		HookDolphinProcess();
		if (DolphinHandle == NULL)
		{
			std::cout << "Unable to hook Dolphin process." << std::endl;
			return -6;
		}

		std::cout << "Started in test mode." << std::endl;
		std::cout << "     Inventory size (in bytes): " << sizeof(WWInventory) << std::endl;
		std::cout << "         Flags size (in bytes): " << sizeof(WWFlags) << std::endl;
		std::cout << "    Total data size (in bytes): " << 2 + sizeof(WWInventory) + sizeof(WWFlags) << std::endl;
		std::cout << "Network buffer size (in bytes): " << WWINV_BUFFER_LENGTH << std::endl << std::endl;

		while (GetCurrentStage() == "sea_T" || GetCurrentStage() == "Name")
		{
			
		}

		string playerName = GetPlayerName();
		std::cout << playerName << " started a game!" << std::endl << std::endl;

		localUserInv = GetInventoryFromProcess();
		localUserFlags = GetFlagsFromProcess();
		PrintInventory(localUserInv);
		
		while (running)
		{
			swapInv = GetInventoryFromProcess();
			if (InvChanged(localUserInv, swapInv))
			{
				patchInv = MakePatch(localUserInv, swapInv);
				PrintInventory(patchInv);
				localUserInv = swapInv;
			}
			
			swapFlags = GetFlagsFromProcess();
			int numFlagsChanged = WWFlagsChanged(localUserFlags, swapFlags);
			
			if (numFlagsChanged > 0)
			{
				std::cout << numFlagsChanged << " flags changed." << std::endl;
				localUserFlags = swapFlags;
			}

			Sleep(WW_INTERVAL);
		}

		CloseHandle(DolphinHandle);
		return 0;
	}
	else
	{
		ShowUsage();
		return 1;
	}

	WSACleanup();
	return 0;
}

void ShowUsage()
{
	std::cout << "Usage:" << std::endl;
	std::cout << "    -s (or -S) <port>                  Create a server on the selected port" << std::endl;
	std::cout << "    -c (or -C) <ipaddress> <port>      Join a server at the selected address and port" << std::endl;
	std::cout << "    -testmode                          Hook the game with no networking. Lists items as you acquire them" << std::endl;
}

UINT NewClientThread(LPVOID newClient)
{
	SOCKET client = (SOCKET)newClient;
	char buffer[WWINV_BUFFER_LENGTH];
	char sendBuffer[WWINV_BUFFER_LENGTH];
	int bytesRead = 0;
	int bytesSent = 0;
	vector<string> itemsList;

	while (running)
	{
		// Send a request for the client's inventory
		SetBufferCommand(sendBuffer, WW_COMMAND_POLL);
		bytesSent = send(client, sendBuffer, 2, 0);
		LogVerbose(" bytes sent to client", bytesSent);

		if (bytesSent == -1)
			goto DISCONNECT_CLIENT;
		
		bytesRead = recv(client, buffer, sizeof(buffer), 0);
		LogVerbose(" bytes received from client", bytesRead);

		if (bytesRead >= (2 + sizeof(WWInventory) + sizeof(WWFlags)))
		{
			short response = 0;
			response = GetBufferCommand(buffer);

			if (response == WW_RESPONSE_POLL)
			{
				// Lazy deserialization
				WWInventory clientInv;
				WWFlags clientFlags;
				memcpy(&clientInv, &buffer[2], sizeof(WWInventory));
				memcpy(&clientFlags, &buffer[2 + sizeof(WWInventory)], sizeof(WWFlags));

				if (InvChanged(localUserInv, clientInv))
				{
					// Update server inventory first
					patchInv = MakePatch(localUserInv, clientInv);

					LogVerbose("Updating server inventory from client inventory");

					PrintInventory(patchInv);

					swapInv = localUserInv;
					swapInv.UpdateInventoryFromPatch(patchInv);
					StoreInventoryToProcess(patchInv);
					localUserInv = swapInv;				
				}

				// Generate a patch for this client
				patchInv = MakePatch(clientInv, localUserInv);

				localUserFlags = GetFlagsFromProcess();

				// Update server flags
				if (WWFlagsChanged(localUserFlags, clientFlags))
				{
					PatchFlags(localUserFlags, clientFlags);
					StoreFlagsToProcess(localUserFlags);
				}

				// Update client flags
				PatchFlags(clientFlags, localUserFlags);

				SetBufferCommand(sendBuffer, WW_COMMAND_SET);

				// Lazy serialization
				memcpy(&sendBuffer[2], &patchInv, sizeof(WWInventory));
				memcpy(&sendBuffer[2 + sizeof(WWInventory)], &clientFlags, sizeof(WWFlags));
				bytesSent = send(client, sendBuffer, 2 + sizeof(WWInventory) + sizeof(WWFlags), 0);

				LogVerbose(" bytes sent to client", bytesSent);
			}
		}
		
		Sleep(WW_INTERVAL);
	}
DISCONNECT_CLIENT:
	LogVerbose("No response from client. Terminating thread.");
	closesocket(client);
	return 0;
}

UINT ListenThread(LPVOID listenerSocket)
{
	SOCKET listener = (SOCKET)listenerSocket;
	while (running)
	{
		SOCKET newConnection = INVALID_SOCKET;
		newConnection = accept((SOCKET)listener, nullptr, nullptr);
		if (newConnection != INVALID_SOCKET)
		{
			std::cout << "New client connected. " << std::endl;
			AfxBeginThread(NewClientThread, (LPVOID)newConnection);
		}
	}

	closesocket(listener);
	return 0;
}
