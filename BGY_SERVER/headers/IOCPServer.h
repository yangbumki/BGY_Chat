#pragma once

#include "../../Common/common.h"
#include "BGYSqlite.h"

//#include <queue>

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

	//공용 데이터 변수
	//std::vector<CLIENT_IO_DATA*> commonDatas;
	std::vector<CLIENT_IO_DATA> commonDatas;

	//서버 외 변수
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

	template <typename T>
	bool SendData(DataHeaders* dh, T* data);

	bool UpdateFriendInfo();

public:
	IOCPServer(u_int serverPort,BGYSqlite* sql);
	~IOCPServer();

	

	bool ServerOpen();
	bool ServerStop();
	bool ServerRestart();

	bool SetParsingData();
};

