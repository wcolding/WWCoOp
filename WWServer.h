#pragma once
#include "WWCore.h"


#define WWINV_BUFFER_LENGTH 64
#define WW_DEFAULT_PORT "2821"

class WWServer
{
public:
	fd_set clientList;
	char buffer[WWINV_BUFFER_LENGTH];
	WWServer(const char* portStr);
	WWServer();

	vector<char*> clientBuffers;

	void Update();

private:
	int iResult;
	SOCKET listener;
	WSADATA wsa;

	int bytesRead = 0;
	void AcceptConnection();
};

class WWClient
{

};