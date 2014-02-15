#include "database.h"

namespace sqlite3db {

int bindArg(Statement &s, uint64_t arg)  {
	return sqlite3_bind_int64(s.stmt, s.pos++, arg);
}

int bindArg(Statement &s, const char *arg)  {
	return sqlite3_bind_text(s.stmt, s.pos++, arg, strlen(arg), nullptr);
}

int bindArg(Statement &s, const std::string &arg)  {
	return sqlite3_bind_text(s.stmt, s.pos++, arg.c_str(), arg.length(), nullptr);
}

template <> int stepper(Statement &s) {
	if(sqlite3_column_type(s.stmt, s.pos) != SQLITE_INTEGER)
		throw db_exception("Not an integer");
	return sqlite3_column_int(s.stmt, s.pos++);
}

template <> uint64_t stepper(Statement &s) {
	if(sqlite3_column_type(s.stmt, s.pos) != SQLITE_INTEGER)
		throw db_exception("Not an integer");
	return sqlite3_column_int(s.stmt, s.pos++);
}

template <> double stepper(Statement &s) {
	if(sqlite3_column_type(s.stmt, s.pos) != SQLITE_FLOAT)
		throw db_exception("Not a double");
	return sqlite3_column_double(s.stmt, s.pos++);
}

template <> const char * stepper(Statement &s) {
	if(sqlite3_column_type(s.stmt, s.pos) != SQLITE_TEXT)
		throw db_exception("Not a string");
	return (const char *)sqlite3_column_text(s.stmt, s.pos++);
}

template <> std::string stepper(Statement &s) {
	if(sqlite3_column_type(s.stmt, s.pos) != SQLITE_TEXT)
		throw db_exception("Not a string");
	return std::string((const char*)sqlite3_column_text(s.stmt, s.pos++));
}

template <> std::vector<uint8_t> stepper(Statement &s) {
	//if(sqlite3_column_type(s.stmt, s.pos) != SQLITE_BLOB)
	//	throw db_exception("Not a blob");
	const void *ptr = sqlite3_column_blob(s.stmt, s.pos);
	int size = sqlite3_column_bytes(s.stmt, s.pos++);

	std::vector<uint8_t> res(size);
	memcpy(&res[0], ptr, size);
	return res;
}

template <> std::vector<uint64_t> stepper(Statement &s) {
	//if(sqlite3_column_type(s.stmt, s.pos) != SQLITE_BLOB)
	//	throw db_exception("Not a blob");
	const void *ptr = sqlite3_column_blob(s.stmt, s.pos);
	int size = sqlite3_column_bytes(s.stmt, s.pos++);

	std::vector<uint64_t> res(size/8);
	memcpy(&res[0], ptr, size);
	return res;
}

}
