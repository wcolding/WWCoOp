#include "WWCore.h"
#include "DolphinHook.h"
#include "WWServer.h"

void ShowUsage();

int main(int argc, char *argv[])
{
	cout << "Wind Waker Co-op " << VERSION << endl << endl;

	struct addrinfo* result = NULL;
	struct addrinfo hints;
	WSADATA wsa;
	int iResult;

	char buffer[WWINV_BUFFER_LENGTH];
	char sendBuffer[WWINV_BUFFER_LENGTH]; 
	memset(&buffer, 0, sizeof(buffer));
	memset(&sendBuffer, 0, sizeof(sendBuffer));
	
	// Server configuration
	if (argv[1] == string("-s") || argv[1] == string("-S"))
	{
		if (argc != 3)
		{
			ShowUsage();
			return -1;
		}

		SOCKET listener;
		fd_set clientList;

		iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (iResult != 0)
			return -2; // WSAStartup failed

		memset(&hints, 0, sizeof(hints));
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
		u_long blockingFlags = 1;
		ioctlsocket(listener, FIONBIO, &blockingFlags);
		listen(listener, SOMAXCONN);

		int hook = HookDolphinProcess();
		if (DolphinHandle == NULL)
		{
			cout << "Unable to hook Dolphin process." << endl;
			return -6;
		}

		WWInventory serverInv, swapInv, patchInv;
		serverInv = GetInventoryFromProcess();

		cout << "Server started on port " << argv[2] << "..." << endl << endl;

		vector<string> itemsList = GetInventoryStrings(serverInv);
		for (int i = 0; i < itemsList.size(); i++)
			cout << itemsList[i] << endl;

		FD_ZERO(&clientList);
		int bytesRead = 0;
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
			newConnection = accept(listener, (struct sockaddr*) &connectedInfo, nullptr);
			if (newConnection != INVALID_SOCKET)
			{
				cout << "New client connected. " << endl;
				FD_SET(newConnection, &clientList);
				char newBuffer[WWINV_BUFFER_LENGTH];
				clientBuffers.push_back(newBuffer);
			}

			fd_set clientCopy = clientList;
			int count = select(0, &clientCopy, nullptr, nullptr, nullptr);

			for (int i = 0; i < count; i++)
			{
				SOCKET curClient = clientCopy.fd_array[i];
				
				
				memset(&sendBuffer, 0, sizeof(sendBuffer));
				sendBuffer[0] = 0x06;
				sendBuffer[1] = 0x09;
				send(curClient, sendBuffer, 2, 0);
				
				bytesRead = recv(curClient, buffer, sizeof(buffer), 0);

				if (bytesRead > 0)
				{
								
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

		// waits for server to send request code (0x0609) and sends local inventory back as a response
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

		/* Listen to first 2 bytes:
		 * 0x0609 - Request for inventory
		 * 0x060A -	Write this incoming patch to the game */

	}
	else
		ShowUsage();
	return 1;
}

void ShowUsage()
{
	cout << "Usage:" << endl;
	cout << "    -s (or -S) <port>                  Create a server on the selected port" << endl;
	cout << "    -c (or -C) <ipaddress> <port>      Join a server at the selected address and port" << endl;
}