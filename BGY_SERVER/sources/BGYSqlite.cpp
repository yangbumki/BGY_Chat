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
	if(stmt != nullptr) sqlite3_finalize(stmt);
	if(db != nullptr) sqlite3_close(db);
}

bool BGYSqlite::GetDuplicatedAccount(AccountInfo ai) {
	const char* sqlCmd = "select id from users";
	auto result = sqlite3_prepare(db, sqlCmd, -1, &stmt, nullptr);
	if (result != SQLITE_OK) {
		WarningMessage("Failed to prepare db");
		return false;
	}

	while(sqlite3_step(stmt) != SQLITE_DONE) {
		int maxCol = sqlite3_column_count(stmt);

		for (int col = 0; col < maxCol; col++) {
			const unsigned char* currentID = sqlite3_column_text(stmt, col);
			auto wideID = ConvertCtoWC((const char*)currentID);

			result = wcsncmp(ai.id, wideID, wcslen(ai.id));
			if (result == 0) {
				WarningMessage("[SQL] : Duplicated account");
				return true;
			}
		}
	}

	return false;
}

bool BGYSqlite::CreateAccount(DB_ACCOUNT_INFO ai) {
	char* sQuote = (char*)"\'";
	char* comma = (char*)",";

	auto id = ConvertWCtoC(ai.id);
	auto name = ConvertWCtoC(ai.name);
	auto passwd = ConvertWCtoC(ai.passwd);
	auto birth = ConvertWCtoC(ai.birth);

	char* sql = (char*)"insert into users values(";
	auto addStr = AddString(17, sql,sQuote, id, sQuote, comma, sQuote, passwd, sQuote, comma, sQuote, name, sQuote, comma, sQuote,birth, sQuote, ")");

	auto result = sqlite3_prepare(db, addStr.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK) {
		WarningMessage("Failed to prepare db");
		return false;
	}

	while (sqlite3_step(stmt) != SQLITE_DONE) {};

	return true;
}