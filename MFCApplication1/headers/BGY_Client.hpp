#include "../pch.h"

typedef enum CONNECTION {
	NONE = 0,
	CONNECT = 1,
	DISCONNECT = 2

}connection;

typedef class BGY_CLIENT {
private:
	WSADATA* wsaData = nullptr;
	SOCKET* serverSock = nullptr;
	sockaddr_in* serverAddr = nullptr;

	ClientModel* cm = nullptr;
	BOOL serverInfoStat = FALSE;

protected:
	bool SetServerInfo(int port, const char* ip) {
		if (serverInfoStat == FALSE) {
			if (serverAddr != nullptr) {
				WarningMessage("Aleady exist server-info");
				delete(serverAddr);
				serverAddr = nullptr;
			}
		}

		serverAddr = new sockaddr_in;
		if (serverAddr == nullptr) {
			WarningMessage("Failed to create server-address");
			return false;
		}

		serverAddr->sin_family = AF_INET;
		serverAddr->sin_port = htons(port);
		inet_pton(AF_INET, ip, &serverAddr->sin_addr);

		serverInfoStat = TRUE;
		//오류 검사 코드 추가하면 좋음

		return true;
	}

public:
	BGY_CLIENT() {
		if (wsaData != nullptr) ErrorMessage("[CLIENT] Aledady exist wsa-data");

		wsaData = new WSADATA;
		if (wsaData == nullptr) ErrorMessage("[CLIENT] Failed to create wsa-data");

		auto result = WSAStartup(MAKEWORD(2, 2), wsaData);
		if (result < 0) ErrorMessage("[CLIENT] Failed to start wsa");

		if (serverSock != nullptr) ErrorMessage("[CLIENT] Aledady exist server-socket");

		serverSock = new SOCKET;
		if (serverSock == nullptr) ErrorMessage("[CLIENT] Failed to create socket");

		*serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (*serverSock == INVALID_SOCKET) ErrorMessage("[CLIENT] FALIED to create tcp-socket");

		cm = new ClientModel;
		memset(cm, 0, sizeof(ClientModel));

		//Set test server info
		SetServerInfo(8986, "127.0.0.1");
	}

	~BGY_CLIENT() {
		WSACleanup();
		delete(wsaData);
	}

	bool ConnectServer() {
		if (serverInfoStat == FALSE) {
			WarningMessage("Is not exist server-info");
			WarningMessage("Plz set server-info");
			return false;
		}

		auto result = connect(*serverSock, (sockaddr*)serverAddr, sizeof(sockaddr_in));
		if (result < 0) {
			WarningMessage("Failed to connect server");
			return false;
		}

		SetColorMessage("Success to connect server", ccolor::LIGHTGREEN, ccolor::BLACK);
		return true;
	}

	template <typename T>
	bool SendData(DataHeaders* header, T* data, int size = 0) {
		//byte 형식에 데이터를 주고 받을 때 size 값 필요하여 추가
		//사이즈가 0 이라면 템플릿 사이즈 받아오기
		if (size == 0) {
			size = sizeof(T);
		}

		if (data == nullptr) {
			WarningMessage("[CLIENT] : Failed to send-data");
			return false;
		}

		//보낼 데이터 초기화
		memset(&cm->clientData, 0, sizeof(cm->clientData));

		//연속적인 데이터를 보낼 경우, Header 미포함으로 전송이 가능 해야함
		if (header != nullptr) {
			memcpy_s(cm->clientData.data, sizeof(cm->clientData.data), header, sizeof(DataHeaders));
			//사이즈 정보 값 받아와서 복사
			//memcpy_s(cm->clientData.data + sizeof(DataHeaders), sizeof(cm->clientData.data), data, sizeof(T));
			memcpy_s(cm->clientData.data + sizeof(DataHeaders), sizeof(cm->clientData.data), data, size);
		}
		//헤더가 포함되어 있지 않을 경우, 헤더 값, 헤더 오프 셋 데이터 미포함
		else if (header == nullptr) {
			memcpy_s(cm->clientData.data, sizeof(cm->clientData.data), data, size);
		}

		cm->clientData.wsaBuf.buf = cm->clientData.data;
		cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

		int result = 0;

		result = WSASend(*this->serverSock, &cm->clientData.wsaBuf, 1, &cm->clientData.sendBytes, cm->clientData.flag, &cm->clientData.overlapped, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
			WarningMessage("[CLIENT] : Failed to send data-header");
			return false;
		}

		return true;
	}

	template <typename T>
	bool GeneralSendData(DataHeaders* header, T* data, int size = 0) {
		//byte 형식에 데이터를 주고 받을 때 size 값 필요하여 추가
		//사이즈가 0 이라면 템플릿 사이즈 받아오기
		if (size == 0) {
			size = sizeof(T);
		}

		if (data == nullptr) {
			WarningMessage("[CLIENT] : Failed to send-data");
			return false;
		}

		//보낼 데이터 초기화
		memset(&cm->clientData, 0, sizeof(cm->clientData));

		//연속적인 데이터를 보낼 경우, Header 미포함으로 전송이 가능 해야함
		if (header != nullptr) {
			memcpy_s(cm->clientData.data, sizeof(cm->clientData.data), header, sizeof(DataHeaders));
			//사이즈 정보 값 받아와서 복사
			//memcpy_s(cm->clientData.data + sizeof(DataHeaders), sizeof(cm->clientData.data), data, sizeof(T));
			memcpy_s(cm->clientData.data + sizeof(DataHeaders), sizeof(cm->clientData.data), data, size);
		}
		//헤더가 포함되어 있지 않을 경우, 헤더 값, 헤더 오프 셋 데이터 미포함
		else if (header == nullptr) {
			memcpy_s(cm->clientData.data, sizeof(cm->clientData.data), data, size);
		}

		cm->clientData.wsaBuf.buf = cm->clientData.data;
		cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

		int result = 0;

		//일반 send 형태
		/*result = WSASend(*this->serverSock, &cm->clientData.wsaBuf, 1, &cm->clientData.sendBytes, cm->clientData.flag, &cm->clientData.overlapped, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
			WarningMessage("[CLIENT] : Failed to send data-header");
			return false;
		}*/

		cm->clientData.sendBytes = send(*this->serverSock, cm->clientData.data, BUFSIZE, 0);
		if (cm->clientData.data <= 0) {
			WarningMessage("[CLIENT] : Falied to general-send");
			return false;
		}

		return true;
	}


	template <typename T>
	int RecvData(std::vector<T*>* data) {
		ZeroMemory(cm, sizeof(ClientModel));
		cm->clientData.wsaBuf.buf = cm->clientData.data;
		cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

		auto result = WSARecv(*serverSock, &cm->clientData.wsaBuf, 1, &cm->clientData.recvBytes, &cm->clientData.flag, NULL, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			ErrorMessage("[CLIENT] : Failed to recv-data");
			return ERROR;
		}

		if (cm->clientData.recvBytes < 0) {
			WarningMessage("[CLIENT] : Failed to recv-data : byte");
			return ERROR;
		}

		DataHeaders* dataHeader;

		dataHeader = (DataHeaders*)&cm->clientData.data;

		switch (dataHeader->dataType) {
		case DATA_TYPE::RESPOND_FRIEND_INFO: {
			FriendInfo* friendInfo, * tmpFIData;


			for (int cnt = 0; cnt < dataHeader->dataCnt; cnt++) {
				friendInfo = (T*)(cm->clientData.data + sizeof(DataHeaders) + sizeof(FriendInfo) * cnt);
				//username 복사해야함
				tmpFIData = new FriendInfo;

				tmpFIData->userID.append(friendInfo->userID);
				tmpFIData->request = friendInfo->request;
				tmpFIData->friending = friendInfo->friending;
				data->push_back(tmpFIData);
			}
			break;
		}

		default:
			break;
		}

		return SUCCESS;
	}

	template <typename T>
	int RecvData(T* data) {
		ZeroMemory(cm, sizeof(ClientModel));
		cm->clientData.wsaBuf.buf = cm->clientData.data;
		cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

		auto result = WSARecv(*serverSock, &cm->clientData.wsaBuf, 1, &cm->clientData.recvBytes, &cm->clientData.flag, NULL, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			ErrorMessage("[CLIENT] : Failed to recv-data");
			return ERROR;
		}

		if (cm->clientData.recvBytes < 0) {
			WarningMessage("[CLIENT] : Failed to recv-data : byte");
			return ERROR;
		}

		DataHeaders* dataHeader;

		dataHeader = (DataHeaders*)&cm->clientData.data;

		T* tmpData = new T;
		ZeroMemory(tmpData, sizeof(T));

		switch (dataHeader->dataType) {
		case DATA_TYPE::RESPOND_FRIEND_INFO: {
			FriendInfo* friendInfo;

			friendInfo = (FriendInfo*)(cm->clientData.data + sizeof(DataHeaders));

			data = tmpData;
			break;
		}
		default:
			memcpy(data, cm->clientData.data, sizeof(T));
			break;
		}

		return SUCCESS;
	}


	int RespondData() {
		ZeroMemory(cm, sizeof(ClientModel));

		cm->clientData.wsaBuf.buf = cm->clientData.data;
		cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

		auto result = WSARecv(*serverSock, &cm->clientData.wsaBuf, 1, &cm->clientData.recvBytes, &cm->clientData.flag, NULL, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			ErrorMessage("[CLIENT] : Failed to recv-respond");
			return ERROR;
		}

		DataHeaders* dh = (DataHeaders*)cm->clientData.data;

		if (dh->dataType == DataType::RESPOND) {
			auto data = (cm->clientData.data + sizeof(DataHeaders));

			if (*data == RespondDataType::SUCCESS) {
				return RespondDataType::SUCCESS;
			}
			else if (*data == RespondDataType::FAIL) {
				return RespondDataType::FAIL;
			}
		}

		WarningMessage("[CLIENT] : Diffrent Data type");
		WarningMessage("[CLIENT] : Check your code");

		return ERROR;
	}

	int GetServerStat() {
		return this->serverInfoStat;
	}

}BgyClient;
