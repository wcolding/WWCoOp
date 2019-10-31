#include "WWServer.h"


WWServer::WWServer(const char *portStr)
{
		struct addrinfo* result = NULL;
		struct addrinfo hints;

		//const char *portStr = to_string(port).c_str();
		
		iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (iResult != 0)
			return; // WSAStartup failed

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		iResult = getaddrinfo(NULL, portStr, &hints, &result);
		if (iResult != 0)
		{
			int error = WSAGetLastError();
			cout << error << endl;
			WSACleanup();
			return;
		}

		listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (listener == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			int error = WSAGetLastError();
			cout << error << endl;
			WSACleanup();
			return;
		}

		iResult = bind(listener, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(listener); 
			int error = WSAGetLastError();
			cout << error << endl;
			WSACleanup();
			return;
		}

		listen(listener, SOMAXCONN);
		FD_ZERO(&clientList);
}

WWServer::WWServer()
{
	WWServer(WW_DEFAULT_PORT);
}

void WWServer::AcceptConnection()
{

	SOCKET newConnection = INVALID_SOCKET;
	newConnection = accept(listener, nullptr, nullptr);
	if (newConnection != INVALID_SOCKET) 
	{
		cout << "New client connected." << endl;
		FD_SET(newConnection, &clientList);
		char newBuffer[WWINV_BUFFER_LENGTH];
		clientBuffers.push_back(newBuffer);
	}
	else
	{
		int error = WSAGetLastError();
		//cout << error << endl;
	}
}

void WWServer::Update()
{
	AcceptConnection();

	fd_set clientCopy = clientList;
	int count = select(0, &clientCopy, nullptr, nullptr, nullptr);
	int i;
	memset(&buffer, 0, sizeof(buffer));

	for (i = 0; i < count; i++)
	{
		SOCKET curClient = clientCopy.fd_array[i];
		bytesRead = recv(curClient, buffer, sizeof(buffer), 0);

		if (bytesRead > 0)
		{
			memcpy(&clientBuffers[i], &buffer, WWINV_BUFFER_LENGTH);
		}
	}
	
}