#include "WWCore.h"
#include "DolphinHook.h"
#include "WWServer.h"
#include "WorldFlags.h"
#include "GiveItemMacros.h"

void ShowUsage();
UINT ListenThread(LPVOID listener);
UINT NewClientThread(LPVOID newClient);
UINT TestModeCommandsThread(LPVOID p);

bool running = true;

Player localPlayer;

WWInventory swapInv, patchInv;
WWFlags localUserFlags, swapFlags;

vector <string> clientNames;

int main(int argc, char *argv[])
{
	std::cout << "Wind Waker Co-op " << VERSION << std::endl << std::endl;

	string enteredName;
	std::cout << "Enter username (limit 15 chars, no spaces): ";
	std::cin >> enteredName;
	std::cout << std::endl;

	localPlayer.SetName(enteredName);

	std::cout << "Hello, " << localPlayer.name << std::endl;
	
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	WSADATA wsa;
	int iResult;

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

		localPlayer.inventory = GetInventoryFromProcess();
		PrintInventory(localPlayer.inventory);

		while (running)
		{
			swapInv = GetInventoryFromProcess();

			if (InvChanged(localPlayer.inventory, swapInv))
			{
				patchInv = MakePatch(localPlayer.inventory, swapInv);
				PrintInventory(patchInv);
				localPlayer.inventory = swapInv;
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

		localPlayer.inventory = GetInventoryFromProcess();
		PrintInventory(localPlayer.inventory);

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
					localPlayer.inventory = GetInventoryFromProcess();
					localPlayer.checksumA = CalculateChecksum(localPlayer.inventory);

					// Lazy serialization
					SetBufferCommand(sendBuffer, WW_RESPONSE_POLL);
					memcpy(&sendBuffer[2], &localPlayer, sizeof(localPlayer));
					send(client, sendBuffer, 2 + sizeof(localPlayer), 0);
					LogVerbose("Player data sent to server");
					break;
				}
				case WW_COMMAND_SET:
				{
					if (bytesRead < 11)
						break;

					// Directly set memory as instructed by the server
					unsigned int address;
					char* data;
					size_t len;
					memcpy(&len, &buffer[bytesRead - 4], 4);
					memcpy(&address, &buffer[2], 4);
					memcpy(&data, &buffer[6], len);
					WriteProcessMemory(DolphinHandle, (LPVOID)(address), &data, len, nullptr);
					break;
				}
				case WW_COMMAND_SET_CHARTS:
				{
					if (bytesRead < 10)
						break;

					// Directly set charts as instructed by the server
					char chartBuffer[8];
					memcpy(&chartBuffer, &buffer[2], sizeof(chartBuffer));
					localPlayer.inventory.Charts = GetChartsFromBuffer(chartBuffer);
					WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::ChartSlot), &chartBuffer, sizeof(chartBuffer), nullptr);
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
			// Wait for a game to be started
		}

		localPlayer.SetName("Test Player");

		std::cout << localPlayer.name << " started a game!" << std::endl << std::endl;

		localPlayer.inventory = GetInventoryFromProcess();
		PrintInventory(localPlayer.inventory);
		
		LocalContext localFlags;
		localFlags.Update();
		string oldStageName = localFlags.stageName;

		std::cout << "Stage: " << oldStageName << std::endl;

		AfxBeginThread(TestModeCommandsThread, &localPlayer);

		

		while (running)
		{
			swapInv = GetInventoryFromProcess();
			if (InvChanged(localPlayer.inventory, swapInv))
			{
				patchInv = MakePatch(localPlayer.inventory, swapInv);
				PrintInventory(patchInv);
				localPlayer.inventory = swapInv;

				localPlayer.checksumA = CalculateChecksum(localPlayer.inventory);
				
				std::cout << "Checksum: " << localPlayer.checksumA << std::endl;
			}

			localFlags.Update();
			if (localFlags.stageName != oldStageName)
			{
				oldStageName = localFlags.stageName;
				std::cout << "Stage: " << oldStageName << std::endl;
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
	Player remotePlayer;
	char buffer[WWINV_BUFFER_LENGTH];
	char sendBuffer[WWINV_BUFFER_LENGTH];
	memset(&buffer, 0, sizeof(buffer));
	memset(&sendBuffer, 0, sizeof(sendBuffer));
	int bytesRead = 0;
	int bytesSent = 0;
	vector<string> itemsList;
	bool announcedPlayer = false;

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

		if (bytesRead >= (2 + sizeof(Player)))
		{
			short response = 0;
			response = GetBufferCommand(buffer);

			if (response == WW_RESPONSE_POLL)
			{
				memcpy(&remotePlayer, &buffer[2], sizeof(remotePlayer));

				if (!announcedPlayer)
					{
						std::cout << remotePlayer.name << " joined the server" << std::endl;
						announcedPlayer = true;
					}

				// Inventory checks
				if (localPlayer.checksumA != remotePlayer.checksumA)
				{
					// Player inventories differ
					// Dump checksums to buffers so we can check what specifically differs
					char localSumBuffer[4], remoteSumBuffer[4];
					memset(&localSumBuffer, 0, 4);
					memset(&remoteSumBuffer, 0, 4);
					memcpy(&localSumBuffer, &localPlayer.checksumA, 4);
					memcpy(&remoteSumBuffer, &remotePlayer.checksumA, 4);

					bool setClient = false;

					if (localSumBuffer[0] != remoteSumBuffer[0])
					{
						// itemStates differ
						for (int i = 0; i < sizeof(localPlayer.inventory.itemStates); i++)
						{
							TestItemStates(client, localPlayer, remotePlayer, i);
						}
					}

					if (localSumBuffer[1] != remoteSumBuffer[1])
					{
						// Songs/Triforce/Pearls/Statues differ
						// Always OR different values and set both server and client to the result
						if (localPlayer.inventory.Songs != remotePlayer.inventory.Songs)
						{
							localPlayer.inventory.Songs |= remotePlayer.inventory.Songs;
							DolphinWrite8(WWItemSlot::SongsSlot, localPlayer.inventory.Songs);
							ClientSetValue(client, WWItemSlot::SongsSlot, &localPlayer.inventory.Songs, 1);
						}

						if (localPlayer.inventory.Triforce != remotePlayer.inventory.Triforce)
						{
							localPlayer.inventory.Triforce |= remotePlayer.inventory.Triforce;
							DolphinWrite8(WWItemSlot::TriforceSlot, localPlayer.inventory.Triforce);
							ClientSetValue(client, WWItemSlot::TriforceSlot, &localPlayer.inventory.Triforce, 1);
						}

						if (localPlayer.inventory.Pearls != remotePlayer.inventory.Pearls)
						{
							localPlayer.inventory.Pearls |= remotePlayer.inventory.Pearls;
							DolphinWrite8(WWItemSlot::PearlSlot, localPlayer.inventory.Pearls);
							ClientSetValue(client, WWItemSlot::PearlSlot, &localPlayer.inventory.Pearls, 1);
						}

						if (localPlayer.inventory.Statues != remotePlayer.inventory.Statues)
						{
							localPlayer.inventory.Statues |= remotePlayer.inventory.Statues;
							DolphinWrite8(WWItemSlot::StatuesSlot, localPlayer.inventory.Statues);
							ClientSetValue(client, WWItemSlot::StatuesSlot, &localPlayer.inventory.Statues, 1);
						}
					}

					if (localSumBuffer[2] != remoteSumBuffer[2])
					{
						// Charts differ
						// Only set client if server has something new
						setClient = (localPlayer.inventory.Charts.GetState() > remotePlayer.inventory.Charts.GetState());
						__int64 newCharts = localPlayer.inventory.Charts.GetState() ^ remotePlayer.inventory.Charts.GetState();
						WWChartState newChartState;
						newChartState.SetState(newCharts);
						char chartBuffer[8];
						SetBufferFromChartState(chartBuffer, newChartState);
						if (!setClient)
						{
							// Set server
							WriteProcessMemory(DolphinHandle, (LPVOID)(BASE_OFFSET + WWItemSlot::ChartSlot), &chartBuffer, sizeof(chartBuffer), nullptr);
						}
						else
						{
							// Set client
							char setChartsBuffer[WWINV_BUFFER_LENGTH];
							SetBufferCommand(setChartsBuffer, WW_COMMAND_SET_CHARTS);
							memcpy(&setChartsBuffer[2], &chartBuffer, sizeof(chartBuffer));
							send(client, setChartsBuffer, sizeof(chartBuffer) + 2, 0);
						}
					}

					if (localSumBuffer[3] != remoteSumBuffer[3])
					{
						// Wallet/Magic/Capacities/Hearts/PoH differ

						if (localPlayer.inventory.Wallet != remotePlayer.inventory.Wallet)
						{
							setClient = (localPlayer.inventory.Wallet > remotePlayer.inventory.Wallet);
							if (!setClient)
							{
								// Update local player
								localPlayer.inventory.Wallet = remotePlayer.inventory.Wallet;
								DolphinWrite8(WWItemSlot::WalletSlot, localPlayer.inventory.Wallet);
								// Print to console?
							}
							else
							{
								// Update remote player
								ClientSetValue(client, WWItemSlot::WalletSlot, &localPlayer.inventory.Wallet, 1);
							}
						}

						if (localPlayer.inventory.Magic != remotePlayer.inventory.Magic)
						{
							setClient = (localPlayer.inventory.Magic > remotePlayer.inventory.Magic);
							if (!setClient)
							{
								// Update local player
								localPlayer.inventory.Magic = remotePlayer.inventory.Magic;
								DolphinWrite8(WWItemSlot::MagicSlot, localPlayer.inventory.Magic);
								// Print to console?
							}
							else
							{
								// Update remote player
								ClientSetValue(client, WWItemSlot::MagicSlot, &localPlayer.inventory.Magic, 1);
							}
						}

						if (localPlayer.inventory.Quiver != remotePlayer.inventory.Quiver)
						{
							setClient = (localPlayer.inventory.Quiver > remotePlayer.inventory.Quiver);
							if (!setClient)
							{
								// Update local player
								localPlayer.inventory.Quiver = remotePlayer.inventory.Quiver;
								DolphinWrite8(WWItemSlot::BowMaxAmmo, localPlayer.inventory.Quiver);
								// Print to console?
							}
							else
							{
								// Update remote player
								ClientSetValue(client, WWItemSlot::BowMaxAmmo, &localPlayer.inventory.Quiver, 1);
							}
						}

						if (localPlayer.inventory.BombBag != remotePlayer.inventory.BombBag)
						{
							setClient = (localPlayer.inventory.BombBag > remotePlayer.inventory.BombBag);
							if (!setClient)
							{
								// Update local player
								localPlayer.inventory.BombBag = remotePlayer.inventory.BombBag;
								DolphinWrite8(WWItemSlot::BombsMaxAmmo, localPlayer.inventory.BombBag);
								// Print to console?
							}
							else
							{
								// Update remote player
								ClientSetValue(client, WWItemSlot::BombsMaxAmmo, &localPlayer.inventory.BombBag, 1);
							}
						}

						if (localPlayer.inventory.Hearts != remotePlayer.inventory.Hearts)
						{
							setClient = (localPlayer.inventory.Hearts > remotePlayer.inventory.Hearts);
							if (!setClient)
							{
								// Update local player
								localPlayer.inventory.Hearts = remotePlayer.inventory.Hearts;
								DolphinWrite8(WWItemSlot::HeartContainers, localPlayer.inventory.Hearts);
								// Print to console?
							}
							else
							{
								// Update remote player
								ClientSetValue(client, WWItemSlot::HeartContainers, &localPlayer.inventory.Hearts, 1);
							}
						}

						if (localPlayer.inventory.PiecesofHeart!= remotePlayer.inventory.PiecesofHeart)
						{
							// PoH not supported yet	
						}


					}
				}
				
				// Flag checks
				if (localPlayer.checksumB != remotePlayer.checksumB)
				{
					// World states differ
				}
				
				//// Lazy deserialization
				//WWInventory clientInv;
				//memcpy(&clientInv, &buffer[2], sizeof(WWInventory));

				//if (InvChanged(localPlayer.inventory, clientInv))
				//{
				//	// Update server inventory first
				//	patchInv = MakePatch(localPlayer.inventory, clientInv);

				//	LogVerbose("Updating server inventory from client inventory");

				//	PrintInventory(patchInv);

				//	swapInv = localPlayer.inventory;
				//	swapInv.UpdateInventoryFromPatch(patchInv);
				//	StoreInventoryToProcess(patchInv);
				//	localPlayer.inventory = swapInv;
				//}

				//// Generate a patch for this client
				//patchInv = MakePatch(clientInv, localPlayer.inventory);
				//SetBufferCommand(sendBuffer, WW_COMMAND_SET);

				//// Lazy serialization
				//memcpy(&sendBuffer[2], &patchInv, sizeof(WWInventory));
				//bytesSent = send(client, sendBuffer, 2 + sizeof(WWInventory), 0);

				//LogVerbose(" bytes sent to client", bytesSent);
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

UINT TestModeCommandsThread(LPVOID p)
{
	Player *thisPlayer = (Player*)p;
	bool checksumKeydown = false;
	while (running)
	{
		if ((GetKeyState('C') & 0x8000) > 0)
		{
			if (!checksumKeydown)
			{
				checksumKeydown = true;
				thisPlayer->checksumA = CalculateChecksum(thisPlayer->inventory);
				std::cout << "Checksum: " << thisPlayer->checksumA << std::endl;
			}
		}
		else
		{
			checksumKeydown = false;
		}

		if ((GetKeyState('1') & 0x8000) > 0)
			SetSword(1);
		if ((GetKeyState('2') & 0x8000) > 0)
			SetSword(2);
		if ((GetKeyState('3') & 0x8000) > 0)
			SetSword(3);
		if ((GetKeyState('4') & 0x8000) > 0)
			SetSword(4);
	}
	
	return 0;
}