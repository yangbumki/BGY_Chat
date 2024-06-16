#pragma once

#include "../../Common/common.h"
#include "BGYSqlite.h"

#define ALEDADY_EXIST		-1
#define ERROR				-1

#define STOP				0
#define DISCONNECTION		64

class IOCPServer
{
private:
	WSADATA* wsaData					= nullptr;
	SOCKET* serverSock					= nullptr;
	SOCKADDR_IN* serverAddr				= nullptr;

	HANDLE iocpHandle					= NULL;
	HANDLE serverOpenThreadHandle		= NULL;
	std::vector<HANDLE> workerThreads;

	std::vector<ClientModel*> cModels;

	BGYSqlite* bgySql = nullptr;

	typedef enum SERVER_STATUS {
		SERVER_NONE = 0,
		SERVER_OPEN,
		SERVER_STOP,
	}serverStat;
	BYTE serverStatus = SERVER_NONE;


private:
	static DWORD WINAPI ServerOpenThread(LPVOID args);
	static DWORD WINAPI ServerWorkerThread(LPVOID args);

	bool CreateWorker(int workerCnt = 0);

public:
	IOCPServer(u_int serverPort,BGYSqlite* sql);
	~IOCPServer();

	bool ServerOpen();
	bool ServerStop();
	bool ServerRestart();

	bool SetParsingData();
	
};

