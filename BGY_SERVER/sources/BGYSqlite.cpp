#include "../headers/BGYSqlite.h"

BGYSqlite::BGYSqlite(const char* dbName) {
	if (db != nullptr) {
		ErrorMessage("Aleady exist db");
	}

	auto result = sqlite3_open(dbName, &db);
	if (result != SQLITE_OK) {
		ErrorMessage("Failed to open db");
	}

	SetColorMessage("[SQL] : Success to Open DB", ccolor::LIGHTGREEN, ccolor::BLACK);
}
BGYSqlite::~BGYSqlite() {
	if (stmt != nullptr) sqlite3_finalize(stmt);
	if (db != nullptr) sqlite3_close(db);
}


//계정 관련 함수
bool BGYSqlite::GetDuplicatedAccount(const DB_ACCOUNT_INFO ai) {
	const char* sqlCmd = "select id from users";
	auto result = sqlite3_prepare(db, sqlCmd, -1, &stmt, nullptr);
	if (result != SQLITE_OK) {
		WarningMessage("Failed to prepare db");
		sqlite3_finalize(stmt);
		return false;
	}

	while (sqlite3_step(stmt) != SQLITE_DONE) {
		int maxCol = sqlite3_column_count(stmt);

		for (int col = 0; col < maxCol; col++) {
			const unsigned char* currentID = sqlite3_column_text(stmt, col);
			auto wideID = ConvertCtoWC((const char*)currentID);

			result = wcsncmp(ai.id, wideID, wcslen(ai.id));
			if (result == 0) {
				WarningMessage("[SQL] : Duplicated account");
				sqlite3_finalize(stmt);
				return true;
			}
		}
	}
	sqlite3_finalize(stmt);
	return false;
}
bool BGYSqlite::CreateAccount(const DB_ACCOUNT_INFO ai) {
	char* sQuote = (char*)"\'";
	char* comma = (char*)",";

	auto id = ConvertWCtoC(ai.id);
	auto name = ConvertWCtoC(ai.name);
	auto passwd = ConvertWCtoC(ai.passwd);
	auto birth = ConvertWCtoC(ai.birth);

	/*char* sql = (char*)"insert into users values(";
	auto addStr = AddString(17, sql,sQuote, id, sQuote, comma, sQuote, passwd, sQuote, comma, sQuote, name, sQuote, comma, sQuote,birth, sQuote, ")");*/

	std::vector<std::string> sql;

	sql.push_back(AddString("insert into users values('%s', '%s', '%s', '%s');", id, passwd, name, birth));
	sql.push_back((AddString("create table friend_%s(users text, friend bool, friend_request bool);", id)));
	//2024-07-15 대화정보 저장 테이블 생성
	sql.push_back((AddString("create table chat_%s(users text,chating text,date text);", id)));

	for (int cnt = 0; cnt < sql.size(); cnt++) {
		auto result = sqlite3_prepare(db, sql[cnt].c_str(), -1, &stmt, nullptr);
		if (result != SQLITE_OK) {
			WarningMessage("Failed to prepare db");
			sqlite3_finalize(stmt);
			return false;
		}
		while (sqlite3_step(stmt) != SQLITE_DONE) {};
		Sleep(DATABASE_INTERVAL_TIME);
		sqlite3_finalize(stmt);
	}

	sql.clear();

	return true;
}
bool BGYSqlite::CheckAccount(const AccountInfo* ai) {
	const char* sqlCmd = "select id, password from users";
	auto result = sqlite3_prepare(db, sqlCmd, -1, &stmt, nullptr);
	if (result != SQLITE_OK) {
		WarningMessage("Failed to prepare db");
		sqlite3_finalize(stmt);
		return false;
	}

	while (sqlite3_step(stmt) != SQLITE_DONE) {
		int maxCol = sqlite3_column_count(stmt);

		for (int col = 0; col < maxCol; col++) {
			const unsigned char* currentID = sqlite3_column_text(stmt, col++);
			const unsigned char* currendPasswd = sqlite3_column_text(stmt, col);

			auto checkID = ConvertCtoWC((const char*)currentID);
			auto checkPasswd = ConvertCtoWC((const char*)currendPasswd);

			result = wcsncmp(ai->id, checkID, wcslen(checkID));
			if (result == 0) {
				result = wcsncmp(ai->passwd, checkPasswd, wcslen(checkPasswd));
				if (result == 0) {
					sqlite3_finalize(stmt);
					return true;
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	return false;
}
bool BGYSqlite::GetAccountInfo(AccountInfo* ai) {

	std::string sql = "select * from users";

	auto result = sqlite3_prepare(this->db, sql.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK) {
		WarningMessage("[SQL] Failed to get account-info");
		sqlite3_finalize(stmt);
		return false;
	}

	while (sqlite3_step(stmt) != SQLITE_DONE) {
		int maxColCnt = sqlite3_column_count(stmt);
		if (maxColCnt <= 0) {
			WarningMessage("[SQL] This data not exist.");
			ai = nullptr;
			sqlite3_finalize(stmt);
			return true;
		}

		for (int col = 0; col < maxColCnt; col++) {
			switch (sqlite3_column_type(stmt, col)) {
			case SQLITE_TEXT: {
				auto currentData = sqlite3_column_text(stmt, col);
				auto parsingData = ConvertCtoWC((const char*)currentData);

				result = wcscmp(parsingData, ai->id);
				if (result == STR_SAME) {
					//name, 순서로 이동
					auto currentData = sqlite3_column_text(stmt, col + 2);
					auto parsingData = ConvertCtoWC((const char*)currentData);
					wcscpy_s(ai->name, parsingData);

					//birth, 순서로 이동
					currentData = sqlite3_column_text(stmt, col + 3);
					parsingData = ConvertCtoWC((const char*)currentData);
					wcscpy_s(ai->birth, parsingData);

					std::cout << "[SQL] Success to find account-info" << std::endl;

					sqlite3_finalize(stmt);
					return true;
				}
				break;
			}
			default:
				break;
			}
		}
	}
	sqlite3_finalize(stmt);
	return false;
}

//친구 관련함수
std::vector<FRIENDINFO> BGYSqlite::GetFriendInfo(const DB_ACCOUNT_INFO ai) {

	std::string sql = AddString("select * from friend_%s", ConvertWCtoC(ai.id));

	auto result = sqlite3_prepare(this->db, sql.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK) {
		WarningMessage("[SQL] Failed to get friend-info");
		sqlite3_finalize(stmt);
		return {};
	}

	std::vector<FRIENDINFO> fi;
	FriendInfo tmpFi;

	while (sqlite3_step(stmt) != SQLITE_DONE) {
		int maxCnt = sqlite3_column_count(stmt);
		void* tmpData = nullptr;

		for (int cnt = 0; cnt < maxCnt; cnt++) {

			auto type = sqlite3_column_type(stmt, cnt);

			switch (type) {
			case SQLITE_TEXT:
				tmpData = (void*)sqlite3_column_text(stmt, cnt);
				break;
			case SQLITE_INTEGER:
				tmpData = (void*)sqlite3_column_int(stmt, cnt);
				break;
			default:
				WarningMessage("[SQL] Somethings wrong friend-adata");
				sqlite3_finalize(stmt);
				return {};
			}

			switch (cnt) {
			case FRIENDINFOTYPE::USERNAME:
				tmpFi.userID = ((char*)tmpData);
				break;
			case FRIENDINFOTYPE::FRINEDING:
				tmpFi.friending = tmpData;
				break;
			case FRIENDINFOTYPE::REQUEST:
				tmpFi.request = tmpData;
			}
		}


		fi.push_back(tmpFi);
	}
	sqlite3_finalize(stmt);

	return fi;
}
bool BGYSqlite::UpdateFriendInfo(const AccountInfo* ai, const FRIENDINFO* fi) {
	std::string sql;

	//친구 거절
	//해당 sql 문에 전달되는 friending, request는 bool 형식이기에 문자열로 판별해야된다.
	//변수 선언
	std::string friStr;
	std::string reqStr;

	const char* trueStr = "1";
	const char* falseStr = "0";


	if (fi->friending) friStr.append(trueStr);
	else friStr.append(falseStr);

	if (fi->request) reqStr.append(trueStr);
	else reqStr.append(falseStr);

	//친구 거절
	if (fi->request == 0 && fi->friending == 0) {
		sql = AddString("delete from friend_%s where users='%s'", ConvertWCtoC(ai->id), fi->userID.c_str());
	}
	//친구 수락
	else {
		sql = AddString("update friend_%s set friend_request=%s, friend=%s where users='%s'", ConvertWCtoC(ai->id), reqStr.c_str(), friStr.c_str(), fi->userID.c_str());
	}

	auto result = sqlite3_prepare(db, sql.c_str(), -1, &stmt, NULL);
	if (result != SQLITE_OK) {
		WarningMessage("[SQL] : Failed to prepare : UpdateFriendInfo");
		sqlite3_finalize(stmt);
		return false;
	}

	while (sqlite3_step(stmt) != SQLITE_DONE);

	sqlite3_finalize(stmt);

	return true;
}
bool BGYSqlite::AddFrindInfo(const AccountInfo* ai, const FRIENDINFO* fi) {
	//친구 중복 확인 변수
	std::vector<FRIENDINFO> checkFriendInfos;
	int totCnt;
	int result;
	//친구 중복 확인
	checkFriendInfos = GetFriendInfo(*ai);
	totCnt = checkFriendInfos.size();
	if (0 < totCnt) {
		for (int cnt = 0; cnt < totCnt; cnt++) {
			//이름으로만 확인
			result = strcmp(checkFriendInfos[cnt].userID.c_str(), fi->userID.c_str());
			if (result == 0) {
				WarningMessage("[SQL] : Failed to AddFriendInfo");
				WarningMessage("[SQL] : Aledy exist Frind-info");
				return false;
			}
		}
	}
	//친구 추가 변수
	std::string sql;

	//친구 추가
	//values 순서 : userName, Frind, Frind_request
	/*sql = AddString("insert into frined_%s values('%s',%s, %s)", ai->id, fi->userID, fi->friending, fi->request);*/
	//해당 sql 문에 전달되는 friending, request는 bool 형식이기에 문자열로 판별해야된다.
	//변수 선언
	std::string friStr;
	std::string reqStr;

	const char* trueStr = "1";
	const char* falseStr = "0";


	if (fi->friending) friStr.append(trueStr);
	else friStr.append(falseStr);

	if (fi->request) reqStr.append(trueStr);
	else reqStr.append(falseStr);
	

	sql = AddString("insert into friend_%s values('%s',%s,%s)", ConvertWCtoC(ai->id), fi->userID.c_str(), friStr.c_str(), reqStr.c_str());

	//Debugging 용 MessageBox 추가
	//MessageBoxA(NULL, sql.c_str(), "SQL", NULL);

	result = sqlite3_prepare(db, sql.c_str(), -1, &stmt, NULL);
	if (result != SQLITE_OK) {
		WarningMessage("[SQL] : Failed to prepare : AddFriendInfo");
		sqlite3_finalize(stmt);
		return false;
	}

	while (sqlite3_step(stmt) != SQLITE_DONE);

	sqlite3_finalize(stmt);

	return true;
}
bool BGYSqlite::AddChatInfo(const AccountInfo* ai, const FRIENDINFO* fi, const ChatInfo* 
chat) {
	std::string sql;

	sql.append(AddString("insert into chat_%s values('%s', '%s', '%s')", fi->userID, chat->userID, chat->chat, chat->date));
	sql.append(AddString("insert into chat_%s values('%s', '%s', '%s')", ai->name, chat->userID, chat->chat, chat->date));

	//이거 맞?
	for (const auto& tmpSql : sql) {
		//sqlite3_prepare(db, tmpSql, -1, &stmt, nullptr);
	}
	return true;
}