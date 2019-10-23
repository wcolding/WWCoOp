#pragma once
#include "WWCore.h"


#define WWINV_BUFFER_LENGTH 32
#define WW_DEFAULT_PORT 2821

class WWServer
{
public:
	fd_set clientList;
	char buffer[WWINV_BUFFER_LENGTH];
	WWServer(int port);

private:
	int iResult;
	SOCKET listener;
	WSADATA wsa;
};

class WWClient
{

};