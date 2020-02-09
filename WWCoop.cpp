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
			swapInv = localPlayer.inventory;

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
					__int8* data;
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
				case WW_COMMAND_GIVE_ITEM:
				{
					if (bytesRead < 3)
						break;

					WWItem rxItem = (WWItem)buffer[2];

					switch (rxItem)
					{
					case WWItem::Telecope:
						GiveTelescope();
						break;
					case WWItem::Sail:
						GiveSail();
						break;
					case WWItem::WW:
						GiveWindWaker();
						break;
					case WWItem::GrapplingHook:
						GiveGrapplingHook();
						break;
					case WWItem::SpoilsBag:
						GiveSpoilsBag();
						break;
					case WWItem::Boomerang:
						GiveBoomerang();
						break;
					case WWItem::DekuLeaf:
						GiveDekuLeaf();
						break;

					case WWItem::TingleTuner:
						GiveTingleTuner();
						break;
					case WWItem::PictoBox1:
						SetPictoBox(1);
						break;
					case WWItem::PictoBox2:
						SetPictoBox(2);
						break;
					case WWItem::Boots:
						GiveIronBoots();
						break;
					case WWItem::MagicArmor:
						GiveMagicArmor();
						break;
					case WWItem::BaitBag:
						GiveHookshot();
						break;
					case WWItem::Bow1:
						SetBow(1);
						break;
					case WWItem::Bow2:
						SetBow(2);
						break;
					case WWItem::Bow3:
						SetBow(3);
						break;
					case WWItem::Bombs:
						GiveBombBag();
						break;

					// Skipping bottles for the moment
					case WWItem::MailBag:
						GiveMailBag();
						break; 
					case WWItem::Hookshot:
						GiveHookshot();
						break;
					case WWItem::Hammer:
						GiveSkullHammer();
						break;

					case WWItem::Sword1:
						SetSword(1);
						break;
					case WWItem::Sword2:
						SetSword(2);
						break;
					case WWItem::Sword3:
						SetSword(3);
						break;
					case WWItem::Sword4:
						SetSword(4);
						break;

					case WWItem::Shield1:
						SetShield(1);
						break;
					case WWItem::Shield2:
						SetShield(2);
						break;

					case WWItem::FathersLetter:
						AddToMail(WWItem::FathersLetter);
						break;
					case WWItem::NotetoMom:
						AddToMail(WWItem::NotetoMom);
						break;
					case WWItem::MaggiesLetter:
						AddToMail(WWItem::MaggiesLetter);
						break;
					case WWItem::MoblinsLetter:
						AddToMail(WWItem::MoblinsLetter);
						break;
					case WWItem::CabanaDeed:
						AddToMail(WWItem::CabanaDeed);
						break;

					default:
						break;
					}

					break;
				}
				case WW_COMMAND_SET_SONGS:
					if (bytesRead < 3)
						break;
					SetSongs(buffer[2]);
					break;
				case WW_COMMAND_SET_TRIFORCE:
					if (bytesRead < 3)
						break;
					SetTriforce(buffer[2]);
					break;
				case WW_COMMAND_SET_PEARLS:
					if (bytesRead < 3)
						break;
					SetPearls(buffer[2]);
					break;
				case WW_COMMAND_SET_STATUES:
					if (bytesRead < 3)
						break;
					SetStatues(buffer[2]);
					break;
				case WW_COMMAND_UPGRADE:
				{
					if (bytesRead < 3)
						break;
					WWUpgradeItem item = (WWUpgradeItem)buffer[2];
					switch (item)
					{
					case ug_Wallet:
						UpgradeWallet();
						break;
					case ug_Magic:
						UpgradeMagic();
						break;
					case ug_Quiver:
						UpgradeQuiver();
						break;
					case ug_BombBag:
						UpgradeBombBag();
						break;
					case ug_Hearts:
						AddHeartContainer();
						break;
					default:
						break;
					}
					break;
				}
				default:
					break;
				}
			}

			// Check for changed inventory and print it to console
			localPlayer.inventory = GetInventoryFromProcess();
			patchInv = MakePatch(swapInv, localPlayer.inventory);
			PrintInventory(patchInv);
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
						std::cout << remotePlayer.name << " connected to the server" << std::endl;
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
							SetSongs(localPlayer.inventory.Songs);
							ClientSetSongs(client, localPlayer.inventory.Songs);
						}

						if (localPlayer.inventory.Triforce != remotePlayer.inventory.Triforce)
						{
							localPlayer.inventory.Triforce |= remotePlayer.inventory.Triforce;
							SetTriforce(localPlayer.inventory.Triforce);
							ClientSetTriforce(client, localPlayer.inventory.Triforce);
						}

						if (localPlayer.inventory.Pearls != remotePlayer.inventory.Pearls)
						{
							localPlayer.inventory.Pearls |= remotePlayer.inventory.Pearls;
							SetPearls(localPlayer.inventory.Pearls);
							ClientSetPearls(client, localPlayer.inventory.Pearls);
						}

						if (localPlayer.inventory.Statues != remotePlayer.inventory.Statues)
						{
							localPlayer.inventory.Statues |= remotePlayer.inventory.Statues;
							SetStatues(localPlayer.inventory.Statues);
							ClientSetPearls(client, localPlayer.inventory.Statues);
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
							}
							else
							{
								// Update remote player
								//ClientSetValue(client, WWItemSlot::WalletSlot, &localPlayer.inventory.Wallet, 1);
								ClientUpgrade(client, WWUpgradeItem::ug_Wallet);
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
							}
							else
							{
								// Update remote player
								//ClientSetValue(client, WWItemSlot::MagicSlot, &localPlayer.inventory.Magic, 1);
								ClientUpgrade(client, WWUpgradeItem::ug_Magic);
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
							}
							else
							{
								// Update remote player
								//ClientSetValue(client, WWItemSlot::BowMaxAmmo, &localPlayer.inventory.Quiver, 1);
								ClientUpgrade(client, WWUpgradeItem::ug_Quiver);
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
							}
							else
							{
								// Update remote player
								//ClientSetValue(client, WWItemSlot::BombsMaxAmmo, &localPlayer.inventory.BombBag, 1);
								ClientUpgrade(client, WWUpgradeItem::ug_BombBag);
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
							}
							else
							{
								// Update remote player
								//ClientSetValue(client, WWItemSlot::HeartContainers, &localPlayer.inventory.Hearts, 1);
								ClientUpgrade(client, WWUpgradeItem::ug_Hearts);
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
	std::cout << remotePlayer.name << " disconnected from the server" << std::endl;
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
	bool walletKey = false;
	bool magicKey = false;
	bool quiverKey = false;
	bool bombKey = false;
	bool heartsKey = false;
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
		{
			if (!walletKey)
			{
				walletKey = true;
				SetPearls(0x07);
			}
		}
		else
		{
			walletKey = false;
		}

		if ((GetKeyState('2') & 0x8000) > 0)
		{
			if (!magicKey)
			{
				magicKey = true;
				UpgradeMagic();
			}
		}
		else
		{
			magicKey = false;
		}

		if ((GetKeyState('3') & 0x8000) > 0)
		{
			if (!quiverKey)
			{
				quiverKey = true;
				UpgradeQuiver();
			}
		}
		else
		{
			quiverKey = false;
		}

		if ((GetKeyState('4') & 0x8000) > 0)
		{
			if (!bombKey)
			{
				bombKey = true;
				UpgradeBombBag();
			}
		}
		else
		{
			bombKey = false;
		}

		if ((GetKeyState('5') & 0x8000) > 0)
		{
			if (!heartsKey)
			{
				heartsKey = true;
				AddHeartContainer();
			}
		}
		else
		{
			heartsKey = false;
		}


	}
	
	return 0;
}