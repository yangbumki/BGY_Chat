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

	if (server->serverStatus != SERVER_NONE) {
		WarningMessage("[IOCP] : Aleady eixst server");
		return ERROR;
	}

	server->CreateWorker();

	//accept
	while (true) {
		cm = new ClientModel;
		memset(cm, 0, sizeof(ClientModel));
		cm->addrLen = sizeof(sockaddr_in);

		cm->socket = accept(*server->serverSock, (sockaddr*)&cm->addr, &cm->addrLen);
		if (cm->socket == INVALID_SOCKET) {
			WarningMessage("Failed to accept client");
			WarningMessage("Check your code : iocp");
			return ERROR;
		}

		server->cModels.push_back(cm);

		server->iocpHandle = CreateIoCompletionPort((HANDLE)cm->socket, server->iocpHandle, (ULONG_PTR)cm, 0);

		cm->clientData.wsaBuf.buf = cm->clientData.data;
		cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

		auto result = WSARecv(cm->socket, &cm->clientData.wsaBuf, 1, &cm->clientData.recvBytes, &cm->clientData.recvBytes, &cm->clientData.overlapped, NULL);

		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			WarningMessage("Failed to recv-data \n");
			WarningMessage("Check your code iocp : serveropen \n");
		}
		SetColorMessage("[IOCP] : Success to accept Client", ccolor::LIGHTGREEN, ccolor::BLACK);

		cm = nullptr;
	}

	return 0;
}

DWORD WINAPI IOCPServer::ServerWorkerThread(LPVOID args) {
	IOCPServer* iocp = (IOCPServer*)args;
	DWORD transBytes = 0;
	ULONG_PTR key = NULL;
	LPOVERLAPPED overlap = nullptr;
	ClientModel* cm = new ClientModel;
	DataHeaders* respondDH = nullptr;
	ClientIOData* respondData = nullptr;

	//pointer 형태로 받기만 할꺼라서, 동적 생성 x
	DataHeaders* header = nullptr;
	AccountInfo* ai = nullptr;


	while (true) {
		auto result = GetQueuedCompletionStatus(iocp->iocpHandle, &transBytes, &key, &overlap, INFINITE);

		if (key == STOP) {
			printf("[IOCP] : Stop Threads \n");
			break;
		}

		if (!result || transBytes <= 0) {
			int error = WSAGetLastError();
			if (error == DISCONNECTION) {
				SetColorMessage("[IOCP] : Abnormal Disconnection", ccolor::RED, ccolor::BLACK);
				continue;
			}
			printf("WSAGetLastError : %d \n", WSAGetLastError());
			continue;
		}

		cm = (ClientModel*)key;

		result = WSARecv(cm->socket, &cm->clientData.wsaBuf, 1, &cm->clientData.recvBytes, &cm->clientData.flag, &cm->clientData.overlapped, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			WarningMessage("[IOCP] : Failed to wsa-recv");
			break;
		}

#if DEBUGGING
		printf("Translate Bytes : %d \n", transBytes);
		printf("Recvied Bytes : %d \n", cm->clientData.recvBytes);
#endif
		

		header = (DataHeaders*)cm->clientData.data;
		char* data = nullptr;

		switch (header->dataType) {
		case CREATE_ACCOUNT:
			ai = (AccountInfo*)(cm->clientData.data + sizeof(DataHeaders));
			if (iocp->bgySql->GetDuplicatedAccount(*ai)) {
				printf("[IOCP] : Aleady exist account \n");
				break;
			}
			if (iocp->bgySql->CreateAccount(*ai)) {
				SetColorMessage("[IOCP] : Success to create account \n", ccolor::YELLOW, ccolor::BLACK);
			}
			break;
		case LOGIN_ACCOUNT:
			ai = (AccountInfo*)(cm->clientData.data + sizeof(DataHeaders));
			//Respond 변수 초기화
			respondData = new ClientIOData;
			ZeroMemory(respondData, sizeof(ClientIOData));
			respondDH = (DataHeaders*)respondData->data;

			respondDH->dataCnt = 1;
			respondDH->dataSize = sizeof(ClientIOData);
			respondDH->dataType = RESPOND;

			data = (char*)(respondData->data + sizeof(DataHeaders));
			//중복 데이터 확인
			if (iocp->bgySql->GetDuplicatedAccount(*ai)) {
				printf("[IOCP] : Login Success \n");
				*data = RESPOND_DATA_TYPE::SUCCESS;
			}
			else *data = RESPOND_DATA_TYPE::FAIL;

			respondData->wsaBuf.buf = respondData->data;
			respondData->wsaBuf.len = sizeof(respondData->data);

			result = WSASend(cm->socket, &respondData->wsaBuf, 1, &respondData->sendBytes, respondData->flag, NULL, NULL);
			if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
				WarningMessage("[IOCP] : Failed to send respond-data");
				break;
			}

			std::cout << "[IOCP] : Success to send respond-data" << std::endl;

			if (respondData != nullptr)
				delete(respondData);
			break;
		case REQUEST_FRIEND_INFO:{
			//Respond 변수 초기화
			respondData = new ClientIOData;
			ZeroMemory(respondData, sizeof(ClientIOData));
			respondDH = (DataHeaders*)respondData->data;

			respondDH->dataCnt = 1;
			respondDH->dataSize = sizeof(ClientIOData);
			respondDH->dataType = RESPOND;

			data = (char*)(respondData->data + sizeof(DataHeaders));
			//중복 데이터 확인
			*data = RESPOND_DATA_TYPE::SUCCESS;

			respondData->wsaBuf.buf = respondData->data;
			respondData->wsaBuf.len = sizeof(respondData->data);

			result = WSASend(cm->socket, &respondData->wsaBuf, 1, &respondData->sendBytes, respondData->flag, NULL, NULL);
			if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
				WarningMessage("[IOCP] : Failed to send respond-data");
				break;
			}

			std::cout << "[IOCP] : Success to send respond-data" << std::endl;

			ZeroMemory(respondData, sizeof(ClientIOData));

			respondData->wsaBuf.buf = respondData->data;
			respondData->wsaBuf.len = sizeof(respondData->data);

			

			ai = (AccountInfo*)(cm->clientData.data + sizeof(DataHeaders));
			auto friendInfos = iocp->bgySql->GetFriendInfo(*ai);

			FriendInfo* friendInfo = nullptr;
			for (int cnt = 0; cnt < friendInfos.size(); cnt++) {
				friendInfo = (FriendInfo*)(respondData->data + sizeof(DataHeaders));
				memcpy(friendInfo, &friendInfos[cnt], sizeof(FriendInfo));
			}

			if (respondData != nullptr)
				delete(respondData);
			break;
		}
		default:
			break;
	}
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
template <typename T>
bool IOCPServer::SendData(DataHeaders* dh, T* data) {
	
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

