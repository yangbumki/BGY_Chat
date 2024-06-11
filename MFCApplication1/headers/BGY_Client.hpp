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

	ClientIOData* clientData = nullptr;
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
	bool SendData(DataHeaders* header, T* data) {
		if (header == nullptr) {
			WarningMessage("[CLIENT] : Failed to send-data");
			return false;
		}

		if (clientData != nullptr) {
			WarningMessage("[CLIENT] : Aleady exist client data");
			WarningMessage("[CLIENT] : Check your code");
			delete(clientData);
			clientData = nullptr;
		}

		clientData = new ClientIOData;
		memset(clientData, 0, sizeof(ClientIOData));

		clientData->wsaBuf.buf = header;
		clientData->wsaBuf.len = sizeof(DataHeaders);

		auto result = WSASend(this->serverSock, clientData->wsaBuf, 1, &clientData->sendBytes, 0, &clientData->overlapped, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
			WarningMessage("[CLIENT] : Failed to send data-header");
			return false;
		}

		ZeroMemory(clientData);
		clientData->wsaBuf = data;
		clientData->wsaBuf.len = header->dataSize;
		auto result = WSASend(this->serverSock, clientData->wsaBuf, 1, &clientData->sendBytes, 0, &clientData->overlapped, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
			WarningMessage("[CLIENT] : Failed to send data");
			return false;
		}

		delete(clientData);
		clientData = nullptr;

		return true;
	}

	int GetServerStat() {
		return this->serverInfoStat;
	}

}BgyClient;
