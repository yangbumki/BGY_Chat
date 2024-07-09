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

	//공통 데이터 변수 정리하기
	//동적 활당 일 경우
	//현재는 벡터 내부 값을 정적 활당으로 변경 하여 상관 없음
	/*int commDataSize = commonDatas.size();
	if (commDataSize > 0) {
		for (int cnt = 0; cnt < commDataSize; cnt++) delete(commonDatas[cnt]);
	}*/
	commonDatas.clear();

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
			//헤더가 없을 경우, 일반데이터 작업
		case COMMON_DATA: {
			iocp->commonDatas.push_back(cm->clientData);
			break;
		}

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

		case LOGIN_ACCOUNT: {
			ai = (AccountInfo*)(cm->clientData.data + sizeof(DataHeaders));
			//Respond 변수 초기화
			respondData = new ClientIOData;
			ZeroMemory(respondData, sizeof(ClientIOData));
			respondDH = (DataHeaders*)respondData->data;

			respondDH->dataCnt = 1;
			respondDH->dataSize = sizeof(ClientIOData);
			respondDH->dataType = RESPOND;

			data = (char*)(respondData->data + sizeof(DataHeaders));
			//아이디 확인
			if (iocp->bgySql->CheckAccount(ai)) {
				printf("[IOCP] : Login Success \n");
				*data = RESPOND_DATA_TYPE::SUCCESS;
			}
			else {
				*data = RESPOND_DATA_TYPE::FAIL;

				respondData->wsaBuf.buf = respondData->data;
				respondData->wsaBuf.len = sizeof(respondData->data);

				result = WSASend(cm->socket, &respondData->wsaBuf, 1, &respondData->sendBytes, respondData->flag, NULL, NULL);
				if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
					WarningMessage("[IOCP] : Failed to send respond-data");
					break;
				}

				if (respondData != nullptr)
					delete(respondData);
				continue;
			}

			respondData->wsaBuf.buf = respondData->data;
			respondData->wsaBuf.len = sizeof(respondData->data);

			result = WSASend(cm->socket, &respondData->wsaBuf, 1, &respondData->sendBytes, respondData->flag, NULL, NULL);
			if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
				WarningMessage("[IOCP] : Failed to send respond-data");
				break;
			}

			//AccountInfo 정보 전달
			if (!iocp->bgySql->GetAccountInfo(ai)) {
				WarningMessage("[IOCP] : Failed to send account-info");
				delete(respondData);
				continue;
			}

			ZeroMemory(respondData, sizeof(ClientIOData));
			respondData->wsaBuf.buf = respondData->data;
			respondData->wsaBuf.len = sizeof(respondData->data);

			memcpy(respondData->data, ai, sizeof(AccountInfo));

			result = WSASend(cm->socket, &respondData->wsaBuf, 1, &respondData->sendBytes, respondData->flag, NULL, NULL);
			if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
				WarningMessage("[IOCP] : Failed to send account-info");
				break;
			}

			std::cout << "[IOCP] : Success to send account-info" << std::endl;

			if (respondData != nullptr)
				delete(respondData);
			break;
		}
		case REQUEST_FRIEND_INFO: {
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
			//친구 목록이 없을 경우
			/*if (friendInfos.size() <= 0) {

			}*/

			respondDH = (DataHeaders*)respondData->data;

			respondDH->dataCnt = friendInfos.size();
			respondDH->dataSize = sizeof(ClientIOData);
			respondDH->dataType = RESPOND_FRIEND_INFO;

			FriendInfo* friendInfo = nullptr;
			for (int cnt = 0; cnt < friendInfos.size(); cnt++) {
				friendInfo = (FriendInfo*)(respondData->data + sizeof(DataHeaders) + sizeof(FriendInfo) * cnt);
				memcpy(friendInfo, &friendInfos[cnt], sizeof(FriendInfo));
			}

			result = WSASend(cm->socket, &respondData->wsaBuf, 1, &respondData->sendBytes, respondData->flag, NULL, NULL);
			if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
				WarningMessage("[IOCP] : Failed to send respond-data");
				break;
			}

			std::cout << "[IOCP] : Success to send friend-info" << std::endl;

			if (respondData != nullptr)
				delete(respondData);
			break;
		}
		case RESPOND_FRIEND_INFO: {
			FriendInfo* recvFriendInfo,* changeFriendInfo;
			AccountInfo* recvAccountInfo,* changeAccountInfo;

			//데이터 동적 활당
			recvAccountInfo = new AccountInfo;
			memset(recvAccountInfo, 0, sizeof(AccountInfo));

			//데이터 복사
			memcpy(recvAccountInfo, (cm->clientData.data + sizeof(DataHeaders)), sizeof(AccountInfo));

			//복사 후 데이터 초기화 : 크기 BUFSIZE
			ZeroMemory(cm->clientData.data, BUFSIZE);
			cm->clientData.recvBytes = 0;
			cm->clientData.sendBytes = 0;

			//클라이언트와 동기화를 위한 데이터 헤더 설정
			//헤더 주소 변경
			DataHeaders* respondDataHeader = (DataHeaders*)cm->clientData.data;
			respondDataHeader->dataType = RESPOND;
			respondDataHeader->dataCnt = 1;
			respondDataHeader->dataSize = BUFSIZE;

			//데이터 주소 설정
			char* respondData = (char*)(respondDataHeader + sizeof(DataHeaders));
			//데이터 설정
			*respondData = SUCCESS;

			//데이터 전송
			result = WSASend(cm->socket, &cm->clientData.wsaBuf, 1, &cm->clientData.sendBytes,cm->clientData.flag, &cm->clientData.overlapped, NULL);
			if (result == SOCKET_ERROR && result != WSA_IO_PENDING) {
				WarningMessage("[IOCP] : Failed to update_friend_info");
				break;
			}

			//데이터 초기화 : 크기 BUFSIZE
			ZeroMemory(cm->clientData.data, BUFSIZE);
			cm->clientData.recvBytes = 0;
			cm->clientData.sendBytes = 0;
			
			//데이터 동기화
			while (cm->clientData.recvBytes == 0 && iocp->commonDatas.size() <= 0);

			//동기화 데이터 초기화
			recvFriendInfo = new FriendInfo;
			memset(recvFriendInfo, 0, sizeof(FriendInfo));

			//동기화 데이터 복사
			memcpy(recvFriendInfo, cm->clientData.data + sizeof(DataHeaders), sizeof(FriendInfo));			

			//내정보 업데이트
			if (iocp->bgySql->UpdateFriendInfo(recvAccountInfo, recvFriendInfo)) {
				changeAccountInfo = new AccountInfo;
				changeFriendInfo = new FriendInfo;

				//이름 정보 교환
				wcscpy_s(changeAccountInfo->id, ConvertCtoWC(recvFriendInfo->userID.c_str()));
				changeFriendInfo->userID.append(ConvertWCtoC(recvAccountInfo->name));

				//친구 정보 변환
				changeFriendInfo->request = recvFriendInfo->request;
				changeFriendInfo->friending = recvFriendInfo->friending;

				//DB Update
				if (!iocp->bgySql->UpdateFriendInfo(changeAccountInfo, changeFriendInfo)) {
					WarningMessage("[IOCP] : Failed to update-friend-info : change");
				}

				//동적 메모리 삭제
				//delete(recvFriendInfo);
				delete(recvAccountInfo);
				delete(changeAccountInfo);
				delete(changeFriendInfo);
			}
			break;
		}
		case ADD_FRIEND: {
			FriendInfo* friendInfo = nullptr,
					* changeFriendInfo = nullptr;

			AccountInfo* accountInfo = nullptr,
					* changeAccountInfo = nullptr;

			//데이터 정보가 BUFSZIE(1024)를 넘기에 두번에 나눠서 받아야 함.
			//첫번째 데이터를 ACcountInfo 형태로, 두번쨰 데이털 FriendInfo 형태로 받을 예정
			
			//받은 데이터를 복사 할 동적 메모리 할당
			accountInfo = new AccountInfo;
			memset(accountInfo, 0, sizeof(AccountInfo));

			//데이터 복사
			memcpy(accountInfo, (cm->clientData.data + sizeof(DataHeaders)), sizeof(AccountInfo));

			//클라이언트와 동기화를 위한 응답 데이터
			//받았던 데이터 초기화
			ZeroMemory(cm->clientData.data, BUFSIZE);
			cm->clientData.recvBytes = 0;
			cm->clientData.sendBytes = 0;

			//응답 데이터 형식 포인터 생성
			DataHeaders* respondDataHeader = nullptr;
			//데이터 헤더 형식 변경
			respondDataHeader = (DataHeaders*)cm->clientData.data;
			//데이터 헤더 설정
			respondDataHeader->dataType = DataType::RESPOND;
			respondDataHeader->dataSize = sizeof(cm->clientData.data);
			respondDataHeader->dataCnt = 1;

			//응답 데이터 형식 포인터 생성
			//데이터 주소 헤더 + 오프셋 값
			char* data = (char*)(respondDataHeader + sizeof(DataHeaders));
			//데이터 설정
			*data = RESPOND_DATA_TYPE::SUCCESS;

			//데이터 전송 설정
			cm->clientData.wsaBuf.buf = cm->clientData.data;
			cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

			//응답 데이터 전송
			result = WSASend(cm->socket, &cm->clientData.wsaBuf, 1, &cm->clientData.sendBytes, cm->clientData.flag, &cm->clientData.overlapped, nullptr);
			if (result == SOCKET_ERROR && result != WSA_IO_PENDING) {
				WarningMessage("[IOCP] : Failed to AddFriend : respond-data");
				break;
			}

			//데이터 초기화
			ZeroMemory(&cm->clientData.data, BUFSIZE);
			cm->clientData.recvBytes = 0;
			cm->clientData.sendBytes = 0;


			//이미 신호를 받아서 처리하는 overlap 구조상 바로 넘어감
			//일반 recv send 구조로 변경
			//WSARecv(cm->socket, &cm->clientData.wsaBuf, 1, &cm->clientData.recvBytes, &cm->clientData.flag, &cm->clientData.overlapped, NULL);
			
			//recv 받기전 다른 비동기 소켓이 먼저 WSARecv로 해당 데이터를 가져가기 떄문에 recv 함수가 따로 동작 하지 않는다.
			//애초에 하나의 데이터를 다 받아온다음 한개로 처리하는 구조로 짰어야 함
			//헤더가 없는 공통된 recv를 위에 따로 빼놓는 방법 시도
			//해당 데이터가 동기화  될 때까지 기다려야함
			//cm->clientData.recvBytes = recv(cm->socket, cm->clientData.data, BUFSIZE, 0);
			//if (cm->clientData.recvBytes <= 0) {
			//	WarningMessage("[IOCP] : Failed to recv-data : ADD_FRIEND");
			//	
			//	//메모리 정리
			//	goto ADD_FRIEND_CLIEAR;
			//}

			//while문으로 무한대기 
			//while (cm->clientData.recvBytes == 0);
			//recv 받음과 데이터 동기화
			while (cm->clientData.recvBytes == 0 && iocp->commonDatas.size() <= 0);
			//GetOverlappedResult함수 사용해보기
			//DWORD trByte;
			//GetOverlappedResult((HANDLE)cm->socket, &cm->clientData.overlapped, &trByte, true);

			//공용 데이터 받아서 처리하기
			// Pointer 형식으로 받을려 하였으나, 포인터 자체가 사라져 원래 데이터로 복사
			//memcpy(&cm->clientData, iocp->commonDatas.back(), sizeof(CLIENT_IO_DATA));
			memcpy(&cm->clientData, &iocp->commonDatas.back(), sizeof(CLIENT_IO_DATA));

			//친구 데이터 복사 할 오적 메모리 할당.
			friendInfo = new FriendInfo;
			memset(friendInfo, 0, sizeof(FriendInfo));

			//데이터 복사
			//데이터 해더 부분 추가
			memcpy(friendInfo, cm->clientData.data+sizeof(DataHeaders), sizeof(FriendInfo));
			
			//받은 데이터 계정정보, 친구정보 변환
			/*accountInfo = (AccountInfo*)(cm->clientData.data + sizeof(DataHeaders));
			friendInfo = (FriendInfo*)(cm->clientData.data + sizeof(DataHeaders) + sizeof(AccountInfo));*/

			if (iocp->bgySql->AddFrindInfo(accountInfo, friendInfo)) {
				changeAccountInfo = new AccountInfo;
				changeFriendInfo = new FriendInfo;

				//ID 상호 복사
				wcscpy_s(changeAccountInfo->id, ConvertCtoWC(friendInfo->userID.c_str()));
				changeFriendInfo->userID.append(ConvertWCtoC(accountInfo->id));
				//친구 정보 복사
				changeFriendInfo->request = true;
				changeFriendInfo->friending = false;

				if (!iocp->bgySql->AddFrindInfo(changeAccountInfo, changeFriendInfo)) {
					WarningMessage("[IOCP] : Failed to AddFrindInfo");
				}

				ADD_FRIEND_CLIEAR:
				//쓰고 남은 잔여 메모리 정리
				//delete(friendInfo);
				delete(accountInfo);
				delete(changeAccountInfo);
				delete(changeFriendInfo);
			}

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

