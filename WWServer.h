#pragma once
#include "WWCore.h"
//#include "Inventory.h"


#define WWINV_BUFFER_LENGTH 64
#define WW_DEFAULT_PORT 2821

class WWServer
{
public:
	fd_set clientList;
	char buffer[WWINV_BUFFER_LENGTH];
	WWServer(int port);

	//vector<WWInventory> clientInvs;

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