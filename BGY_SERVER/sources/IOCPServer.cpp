#include "../headers/IOCPServer.h"

IOCPServer::IOCPServer(u_int serverPort, BGYSqlite* sql)
	: bgySql(sql) {
	if (bgySql == nullptr) {
		ErrorMessage("[IOCP] Failed to initialize sql");
	}

	if (wsaData != nullptr) {
		WarningMessage("Somethings wrong");
		ErrorMessage("Aleady exist wsa-data");
	}

	wsaData = new WSADATA;
	if (wsaData == nullptr) {
		ErrorMessage("Faild to initialize wsa-data");
	}

	auto result = WSAStartup(MAKEWORD(2, 2), wsaData);
	if (result != 0) {
		ErrorMessage("Faild to initialize wsa-data");
	}

	if (serverSock != nullptr) {
		WarningMessage("Somethings wrong");
		ErrorMessage("Aleady exist server-socket");
	}

	serverSock = new SOCKET;
	if (serverSock == nullptr) {
		ErrorMessage("Faild to initialize server-socket");
	}

	*serverSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

	//*serverSock = socket(AF_INET, SOCK_STREAM, 0);

	if (*serverSock == INVALID_SOCKET) {
		ErrorMessage("Failed to create socket");
	}

	if (serverAddr != nullptr) {
		WarningMessage("Somethings wrong");
		ErrorMessage("Aleady eixst server-address");
	}

	serverAddr = new SOCKADDR_IN;
	memset(serverAddr, 0, sizeof(SOCKADDR_IN));

	serverAddr->sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr->sin_family = AF_INET;
	serverAddr->sin_port = htons(serverPort);

	result = bind(*serverSock, (sockaddr*)serverAddr, sizeof(sockaddr));
	if (result != 0) {
		ErrorMessage("Failed to bind socket");
	}

	result = listen(*serverSock, 0);
	if (result < 0) {
		ErrorMessage("Failed to listen socket");
	}
}

IOCPServer::~IOCPServer() {
	closesocket(*serverSock);

	for (int sockCnt = 0; sockCnt < cModels.size(); sockCnt++) {
		closesocket(cModels[sockCnt]->socket);
		delete(cModels[sockCnt]);
	}

	for (int threadCnt = 0; threadCnt < workerThreads.size(); threadCnt++) {
		TerminateThread(workerThreads[threadCnt], 0);
		CloseHandle(workerThreads[threadCnt]);
	}

	if (serverOpenThreadHandle != NULL) {
		TerminateThread(serverOpenThreadHandle, 0);
		CloseHandle(serverOpenThreadHandle);
	}

	WSACleanup();


	delete(wsaData);
	delete(serverSock);
	delete(serverAddr);
}

DWORD WINAPI IOCPServer::ServerOpenThread(LPVOID args) {
	IOCPServer* server = (IOCPServer*)args;
	ClientModel* cm = nullptr;
	ClientIOData* cd = nullptr;
	DataHeaders* dataHeader = nullptr;

	if (server->serverStatus != SERVER_NONE) {
		WarningMessage("[IOCP] : Aleady eixst server");
		return ERROR;
	}

	server->CreateWorker(1);

	//accept
	while (true) {
		cm = new ClientModel;
		memset(cm, 0, sizeof(ClientModel));
		cm->addrLen = sizeof(sockaddr_in);
		cd = new ClientIOData;
		memset(cd, 0, sizeof(ClientIOData));

		cm->socket = accept(*server->serverSock, (sockaddr*)&cm->addr, &cm->addrLen);
		if (cm->socket == INVALID_SOCKET) {
			WarningMessage("Failed to accept client");
			WarningMessage("Check your code : iocp");
			return ERROR;
		}

		server->cModels.push_back(cm);

		server->iocpHandle = CreateIoCompletionPort((HANDLE)cm->socket, server->iocpHandle, (ULONG_PTR)cm, 0);

		cd->wsaBuf.buf = cd->data;
		cd->wsaBuf.len = sizeof(cd->data);
		cd->recvBytes = 0;
		ZeroMemory(&cd->overlapped, sizeof(WSAOVERLAPPED));

		DWORD flag = 0;

		auto result = WSARecv(cm->socket, &cd->wsaBuf, 1, &cd->recvBytes, &flag, &cd->overlapped, NULL);

		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			WarningMessage("Failed to recv-data \n");
			WarningMessage("Check your code iocp : serveropen \n");
		}
		SetColorMessage("[IOCP] : Success to accept Client", ccolor::LIGHTGREEN, ccolor::BLACK);

		cm = nullptr;

		delete(cd);
		cd = nullptr;

		delete(dataHeader);
		dataHeader = nullptr;
	}

	return 0;
}

DWORD WINAPI IOCPServer::ServerWorkerThread(LPVOID args) {
	IOCPServer* iocp = (IOCPServer*)args;
	DWORD transBytes = 0;
	ULONG_PTR key = NULL;
	ClientModel* cm = new ClientModel;
	ClientIOData* cd = new ClientIOData;
	DataHeaders* dataHeader = new DataHeaders;

	ZeroMemory(&cd->overlapped, sizeof(WSAOVERLAPPED));

	while (true) {
		auto result = GetQueuedCompletionStatus(iocp->iocpHandle, &transBytes, &key, (LPOVERLAPPED*)cd, INFINITE);

		if (key == STOP) {
			printf("[IOCP] : Stop Threads \n");
			break;
		}

		if (!result) {
			int error = WSAGetLastError();
			if (error == DISCONNECTION) {
				SetColorMessage("[IOCP] : AbnormaZl Disconnection", ccolor::RED, ccolor::BLACK);
				break;
			}
			printf("WSAGetLastError : %d \n", WSAGetLastError());
			continue;
		}
		cm = (ClientModel*)key;

		
		cd->wsaBuf.buf = cd->data;
		cd->wsaBuf.len = sizeof(cd->data);
		cd->recvBytes = 0;
		cd->flag = 0;
		
		result = WSARecv(cm->socket, &cd->wsaBuf, 1, &cd->recvBytes, &cd->flag, &cd->overlapped, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			continue;
		}

		if (cd->wsaBuf.len <= 0 || cd->recvBytes <= 0) continue;

		dataHeader = (DataHeaders*)cd->data;
		printf("%d \n", dataHeader->dataCnt);
		printf("%d \n", dataHeader->dataSize);
		printf("%d \n", dataHeader->dataType);
		AccountInfo* ai =(AccountInfo*)(cd->data + sizeof(DataHeaders));
		wprintf(L"%s \n",ai->birth);
		wprintf(L"%s \n", ai->name);
		wprintf(L"%s \n", ai->id);
		wprintf(L"%s \n", ai->passwd);
		memset(cd->data, 0, sizeof(cd->data));
		//iocp->SetParsingData();
	}

	return 0;
}

bool IOCPServer::CreateWorker(int workerCnt) {
	if (this->serverStatus == SERVER_OPEN) {
		WarningMessage("[IOCP] : Aleady exist worker");
		return false;
	}
	HANDLE tempThread = NULL;

	if (workerCnt == 0) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		workerCnt = si.dwNumberOfProcessors;
	}

	for (int psCnt = 0; psCnt < workerCnt; psCnt++) {
		tempThread = CreateThread(NULL, 0, ServerWorkerThread, this, NULL, NULL);
		if (tempThread == NULL) {
			ErrorMessage("Failed to create-thread");
			return ERROR;
		}
		workerThreads.push_back(tempThread);
		tempThread = NULL;
	}

	return true;
}
bool IOCPServer::ServerOpen() {
	if (!cModels.empty()) {
		WarningMessage("Aleady eixst clients");
		return false;
	}

	if (iocpHandle != NULL) {
		WarningMessage("Aleady exist iocp");
		return false;
	}

	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	if (iocpHandle == NULL) {
		WarningMessage("Failed to create iocp");
		return false;
	}

	if (serverOpenThreadHandle != NULL) {
		WarningMessage("Aleady exist server");
		TerminateThread(serverOpenThreadHandle, ALEDADY_EXIST);
		CloseHandle(serverOpenThreadHandle);
		serverOpenThreadHandle = NULL;
	}

	serverOpenThreadHandle = CreateThread(NULL, 0, this->ServerOpenThread, this, NULL, NULL);
	if (serverOpenThreadHandle == NULL) {
		WarningMessage("Failed to create thread");
		return false;
	}

	return true;
}
bool IOCPServer::ServerStop() {
	if (workerThreads.size() < 0) {
		WarningMessage("[IOCP] : The thread does not exist.");
		return false;
	}

	for (HANDLE thread : workerThreads) {
		PostQueuedCompletionStatus(this->iocpHandle, 0, 0, NULL);
	}

	WaitForMultipleObjects(workerThreads.size(), workerThreads.data(), TRUE, INFINITE);

	return true;
}
bool IOCPServer::ServerRestart() {
	if (serverStatus == SERVER_OPEN) {
		WarningMessage("[IOCP] : Aleady open Server");
		return false;
	}

	CreateWorker();
	return true;
}

bool IOCPServer::SetParsingData() {
	return true;
}

