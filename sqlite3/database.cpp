#include "database.h"
#include <stdexcept>

namespace sqlite3db {

int bindArg(Statement &s, int64_t arg)  {
	return sqlite3_bind_int64(s.stmt, s.pos++, arg);
}

int bindArg(Statement &s, const char *arg)  {
	return sqlite3_bind_text(s.stmt, s.pos++, arg, strlen(arg), nullptr);
}

int bindArg(Statement &s, const std::string &arg)  {
	return sqlite3_bind_text(s.stmt, s.pos++, arg.c_str(), arg.length(), nullptr);
}

#define START() \
	auto t = sqlite3_column_type(s.stmt, s.pos); \
	LOGD("POSITION %d TYPE %d", s.pos, t); \
	if(t == SQLITE_NULL) \
		throw db_exception("null value"); \

template <> int stepper(Statement &s) {
	START()
	LOGD("int");
	if(t != SQLITE_INTEGER)
		throw std::invalid_argument("Not an integer");
	return sqlite3_column_int(s.stmt, s.pos++);
}

template <> uint64_t stepper(Statement &s) {
	START();
	LOGD("uint64_t");
	if(t != SQLITE_INTEGER)
		throw std::invalid_argument("Not an integer");
	return sqlite3_column_int(s.stmt, s.pos++);
}

template <> double stepper(Statement &s) {
	START()
	if(t != SQLITE_FLOAT)
		throw std::invalid_argument("Not a double");
	return sqlite3_column_double(s.stmt, s.pos++);
}

template <> const char * stepper(Statement &s) {
	START()
	if(t != SQLITE_TEXT)
		throw std::invalid_argument("Not a string");
	return (const char *)sqlite3_column_text(s.stmt, s.pos++);
}

template <> std::string stepper(Statement &s) {
	START()
	LOGD("string");
	if(t != SQLITE_TEXT)
		throw std::invalid_argument("Not a string");
	return std::string((const char*)sqlite3_column_text(s.stmt, s.pos++));
}

template <> std::vector<uint8_t> stepper(Statement &s) {
	START()
	//if(t != SQLITE_BLOB)
	//	throw std::invalid_argument("Not a blob");
	const void *ptr = sqlite3_column_blob(s.stmt, s.pos);
	int size = sqlite3_column_bytes(s.stmt, s.pos++);

	std::vector<uint8_t> res(size);
	memcpy(&res[0], ptr, size);
	return res;
}

template <> std::vector<uint64_t> stepper(Statement &s) {
	START()
	//if(t != SQLITE_BLOB)
	//	throw std::invalid_argument("Not a blob");
	const void *ptr = sqlite3_column_blob(s.stmt, s.pos);
	int size = sqlite3_column_bytes(s.stmt, s.pos++);

	std::vector<uint64_t> res(size/8);
	memcpy(&res[0], ptr, size);
	return res;
}

}
