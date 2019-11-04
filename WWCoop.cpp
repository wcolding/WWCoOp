#include "WWCore.h"
#include "DolphinHook.h"
#include "WWServer.h"

void ShowUsage();

int main(int argc, char *argv[])
{
	cout << "Wind Waker Co-op " << VERSION << endl << endl;

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
		fd_set clientList, clientCopy;

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
			cout << error << endl;
			WSACleanup();
			return -3;
		}

		listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (listener == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			int error = WSAGetLastError();
			cout << error << endl;
			WSACleanup();
			return -4;
		}

		iResult = bind(listener, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(listener);
			int error = WSAGetLastError();
			cout << error << endl;
			WSACleanup();
			return -5;
		}

		ioctlsocket(listener, FIONBIO, &blockingFlags);
		listen(listener, SOMAXCONN);

		HookDolphinProcess();
		if (DolphinHandle == NULL)
		{
			cout << "Unable to hook Dolphin process." << endl;
			return -6;
		}

		WWInventory serverInv, swapInv, patchInv;
		serverInv = GetInventoryFromProcess();

		cout << "Server started on port " << argv[2] << "..." << endl << endl;

		itemsList = GetInventoryStrings(serverInv);
		for (int i = 0; i < itemsList.size(); i++)
			cout << itemsList[i] << endl;

		FD_ZERO(&clientList);
		vector<char*> clientBuffers;
		sockaddr_in connectedInfo;

		while (true)
		{
			swapInv = GetInventoryFromProcess();
			if (InvChanged(serverInv, swapInv))
			{
				patchInv = MakePatch(serverInv, swapInv);
				itemsList = GetInventoryStrings(patchInv);
				for (int i = 0; i < itemsList.size(); i++)
					cout << itemsList[i] << endl;
				serverInv = swapInv;
			}

			SOCKET newConnection = INVALID_SOCKET;
			ioctlsocket(newConnection, FIONBIO, &blockingFlags); // May need to set SO_REUSEADDR
			newConnection = accept(listener, (struct sockaddr*) &connectedInfo, nullptr);
			if (newConnection != INVALID_SOCKET)
			{
				cout << "New client connected. " << endl;
				FD_SET(newConnection, &clientList);
				char newBuffer[WWINV_BUFFER_LENGTH];
				clientBuffers.push_back(newBuffer);
			}

			memset(&clientCopy, 0, sizeof(clientCopy));
			clientCopy = clientList;
			
			int count = select(0, &clientCopy, nullptr, nullptr, &timeout);

			//int count = clientCopy.fd_count;
			if (count > 0)
			{ 
				for (int i = 0; i < count; i++)
				{
					SOCKET curClient = clientCopy.fd_array[i];

					// Send a request for the client's inventory
					SetBufferCommand(buffer, WW_COMMAND_POLL);
					send(curClient, sendBuffer, 2, 0);

					bytesRead = recv(curClient, buffer, sizeof(buffer), 0);

					if (bytesRead >= sizeof(WWInventory))
					{
						// Lazy deserialization
						WWInventory clientInv;
						memcpy(&clientInv, &buffer, sizeof(WWInventory));

						if (InvChanged(serverInv, clientInv))
						{
							// Update server inventory first
							patchInv = MakePatch(serverInv, clientInv);
							itemsList = GetInventoryStrings(patchInv);
							for (int i = 0; i < itemsList.size(); i++)
								cout << itemsList[i] << endl;

							swapInv = serverInv;
							swapInv.UpdateInventoryFromPatch(patchInv);
							StoreInventoryToProcess(patchInv);
							serverInv = swapInv;

							if (InvChanged(clientInv, serverInv))
							{
								// Generate and send a patch for this client
								patchInv = MakePatch(clientInv, serverInv);
								SetBufferCommand(sendBuffer, WW_COMMAND_SET);

								// Lazy serialization
								memcpy(&sendBuffer[2], &patchInv, sizeof(WWInventory));
								send(curClient, sendBuffer, sizeof(WWInventory) + 2, 0);
							}
						}
					}
				}
				Sleep(WW_INTERVAL);
			}
		}
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
			cout << error << endl;
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
				cout << error << endl;
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

		ioctlsocket(client, FIONBIO, &blockingFlags);

		HookDolphinProcess();
		if (DolphinHandle == NULL)
		{
			cout << "Unable to hook Dolphin process." << endl;
			return -6;
		}

		/* Listen to first 2 bytes:
		 * 0x0609 - Request for inventory
		 * 0x060A -	Write this incoming patch to the game */


		WWInventory myInventory, rxPatch;

		myInventory = GetInventoryFromProcess();
		itemsList = GetInventoryStrings(myInventory);
		for (int i = 0; i < itemsList.size(); i++)
			cout << itemsList[i] << endl;

		while (true)
		{
			memset(&buffer, 0, sizeof(buffer));
			memset(&sendBuffer, 0, sizeof(sendBuffer));
			bytesRead = recv(client, buffer, sizeof(buffer), 0);

			if (bytesRead >= 2)
			{
				short command = 0;
				command = GetBufferCommand(buffer);
				
				switch(command)
				{
				case WW_COMMAND_POLL:
				{
					myInventory = GetInventoryFromProcess();
					
					// Lazy serialization
					memcpy(&sendBuffer, &myInventory, sizeof(WWInventory));
					send(client, sendBuffer, sizeof(WWInventory), 0);
					break;
				}
				case WW_COMMAND_SET:
				{
					// Lazy deserialization
					memcpy(&rxPatch, &buffer[2], sizeof(WWInventory));
					itemsList = GetInventoryStrings(rxPatch);
					for (int i = 0; i < itemsList.size(); i++)
						cout << itemsList[i] << endl;
					StoreInventoryToProcess(rxPatch);
					break;
				}
				default:
					break;
				}
			}
		}

	}
	else
	{
		ShowUsage();
		return 1;
	}

	return 0;
}

void ShowUsage()
{
	cout << "Usage:" << endl;
	cout << "    -s (or -S) <port>                  Create a server on the selected port" << endl;
	cout << "    -c (or -C) <ipaddress> <port>      Join a server at the selected address and port" << endl;
}