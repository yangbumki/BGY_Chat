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
	if(stat != nullptr) sqlite3_finalize(stmt);
	if(db != nullptr) sqlite3_close(db);
}

bool BGYSqlite::GetDuplicatedAccount(AccountInfo ai) {
	const char* sqlCmd = "select id from account";
	auto result = sqlite3_prepare(db, sqlCmd, sizeof(sqlCmd), &stmt, nullptr);
	if (result != SQLITE_OK) {
		WarningMessage("Failed to prepare db");
		return false;
	}

	while (sqlite3_step(stmt) != SQLITE_DONE) {
		int maxCol = sqlite3_column_count(stmt);

		for (int col = 0; col < maxCol; col++) {
			const unsigned char* currentID = sqlite3_column_text(stmt, col);
			result = strncmp((const char*)ai.id, (const char*)currentID, sizeof(currentID));
			if (result == 0) {
				WarningMessage("[SQL] : Duplicated account");
				return false;
			}
		}
	}

	return true;
}