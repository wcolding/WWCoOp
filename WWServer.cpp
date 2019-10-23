#include "WWServer.h"


WWServer::WWServer(int port = WW_DEFAULT_PORT)
{
		struct addrinfo* result = NULL;
		struct addrinfo hints;
		
		iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (iResult != 0)
			return; // WSAStartup failed

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		iResult = getaddrinfo(NULL, (PCSTR)WW_DEFAULT_PORT, &hints, &result);
		if (iResult != 0)
		{
			WSACleanup();
			return;
		}

		listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (listener == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			WSACleanup();
			return;
		}

		iResult = bind(listener, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(listener);
			WSACleanup();
			return;
		}

		listen(listener, SOMAXCONN);
		FD_ZERO(&clientList);
}
