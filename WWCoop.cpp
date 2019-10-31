#include "WWCore.h"
#include "DolphinHook.h"
#include "WWServer.h"

int main()
{
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	WSADATA wsa;
	int iResult;

	SOCKET listener;
	fd_set clientList;

	//const char *portStr = to_string(port).c_str();

	iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (iResult != 0)
		return -1; // WSAStartup failed

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, WW_DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		int error = WSAGetLastError();
		cout << error << endl;
		WSACleanup();
		return -2;
	}

	listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listener == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		int error = WSAGetLastError();
		cout << error << endl;
		WSACleanup();
		return -3;
	}

	iResult = bind(listener, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(listener);
		int error = WSAGetLastError();
		cout << error << endl;
		WSACleanup();
		return -4;
	}

	listen(listener, SOMAXCONN);
	FD_ZERO(&clientList);

	while (true)
	{
		SOCKET newConnection = INVALID_SOCKET;
		newConnection = accept(listener, nullptr, nullptr);
		if (newConnection != INVALID_SOCKET)
		{
			cout << "New client connected." << endl;
			FD_SET(newConnection, &clientList);
			char newBuffer[WWINV_BUFFER_LENGTH];
			//clientBuffers.push_back(newBuffer);
		}
	}
	return 0;
}