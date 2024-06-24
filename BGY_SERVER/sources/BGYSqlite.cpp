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

	sql.push_back(AddString("insert into users values('%s', '%s', '%s', '%s');", id, name, passwd, birth));
	sql.push_back((AddString("create table friend_%s(users text, friend bool, friend_request bool);", id)));

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

std::vector<FRIENDINFO> BGYSqlite::GetFriendInfo(const DB_ACCOUNT_INFO ai) {
	
	std::string sql = AddString("select * from friend_%s", ai.name);

	auto result = sqlite3_prepare(this->db, sql.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK) {
		WarningMessage("[SQL] Failed to get friend-info");
		return (std::vector<FRIENDINFO>)ERROR;
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
				return (std::vector<FRIENDINFO>)ERROR;
			}
						
			if (tmpData != nullptr) {
				switch (cnt) {
				case FRIENDINFOTYPE::USERNAME:
					tmpFi.userName = (char*)tmpData;
					break;
				case FRIENDINFOTYPE::FRINEDING:
					tmpFi.friending = tmpData;
					break;
				case FRIENDINFOTYPE::REQUEST:
					tmpFi.request = tmpData;
				}
			}

			if (cnt == FRIENDINFOTYPE::TOTALCNT) {
				fi.push_back(tmpFi);
				ZeroMemory(&tmpFi, sizeof(FriendInfo));
			}
		}
	}

	return fi;
}