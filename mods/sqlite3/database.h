#ifndef SQLITE_DATABASE_H
#define SQLITE_DATABASE_H

#include "sqlite3.h"
#include <coreutils/utils.h>
#include <coreutils/log.h>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <tuple>

namespace sqlite3db {


class db_exception : public std::exception {
public:
	db_exception(const char *ptr = "DB Exception") : msg(ptr) {
	}
	virtual const char *what() const throw() { return msg; }
 private:
 	const char *msg;
};

struct Statement {
	Statement(sqlite3_stmt *s, int p) : stmt(s), pos(p) {}
	sqlite3_stmt *stmt;
	int pos;
};

int bindArg(Statement &s, int64_t arg);

// We need enable_if since otherwise we get ambiguity with the integer version
template <class T> int bindArg(Statement &s, typename std::enable_if<std::is_integral<T>::value, double>::type arg)  {
	return sqlite3_bind_double(s.stmt, s.pos++, arg);
}
int bindArg(Statement &s, const char *arg);
int bindArg(Statement &s, const std::string &arg);

template <typename T> int bindArg(Statement &s, const std::vector<T> &arg)  {
	return sqlite3_bind_blob(s.stmt, s.pos++, &arg[0], arg.size() * sizeof(T), nullptr);
}

// Default stepper signals error
template <typename T> T stepper(sqlite3_stmt *, int) {
	throw db_exception("Uknown target type");
}

// These are the the types we support in our select target
template <> int stepper(sqlite3_stmt *s, int pos);
template <> uint64_t stepper(sqlite3_stmt *s, int pos);
template <> double stepper(sqlite3_stmt *s, int pos);
template <> const char * stepper(sqlite3_stmt *s, int pos);
template <> std::string stepper(sqlite3_stmt *s, int pos);
template <> std::vector<uint8_t> stepper(sqlite3_stmt *s, int pos);
template <> std::vector<uint64_t> stepper(sqlite3_stmt *s, int pos);

class Database {

struct base_query {
	template <class... A> base_query(sqlite3 *db, const std::string &query, const A& ... args) : db(db), stmt(nullptr), lastCode(-1) {
		int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		//LOGD("CONSTRUCT stmt %p", stmt);

		Statement s { stmt, 1 };
		std::vector<int> results = { bindArg(s, args)... };

		for(int &r : results) {
			if(r != SQLITE_OK)
				throw db_exception(sqlite3_errmsg(db));
		}
	};

	template <class A> base_query(sqlite3 *db, const std::string &query, const std::vector<A> &args) : db(db), stmt(nullptr), lastCode(-1) {
		int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		//LOGD("CONSTRUCT stmt %p", stmt);

		Statement s { stmt, 1 };
		std::vector<int> results;
		for(const auto &a : args)
			results.push_back(bindArg(s, a));

		for(int &r : results) {
			if(r != SQLITE_OK)
				throw db_exception(sqlite3_errmsg(db));
		}
	};

	template <class... A> base_query& bind(const A& ... args) {
		sqlite3_clear_bindings(stmt);
    	sqlite3_reset(stmt);

		Statement s { stmt, 1 };
		std::vector<int> results = { bindArg(s, args)... };

		for(int &r : results) {
			if(r != SQLITE_OK)
				throw db_exception(sqlite3_errmsg(db));
		}
		return *this;
    }

	base_query& operator=(const base_query &q) = delete;
	base_query(const base_query &q) = delete;

	base_query(base_query &&q) {
		//LOGD("Copy cons");
		db = q.db;
		stmt = q.stmt;
		lastCode = q.lastCode;
		q.stmt = nullptr;
	}

	base_query& operator=(base_query &&q) {

		finalize();

		//LOGD("Assign %p", q.stmt);
		db = q.db;
		stmt = q.stmt;
		lastCode = q.lastCode;
		q.stmt = nullptr;
		return *this;
	};

	~base_query() {
		//LOGD("Destroy %p", stmt);
		finalize();
	}

	void finalize() {
		//LOGD("finalize %p", stmt);
		if(stmt)
			sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	bool step() {
		lastCode = sqlite3_step(stmt);
		return (lastCode == SQLITE_ROW);
	}

	sqlite3 *db;
	sqlite3_stmt *stmt;
	int lastCode;
};


template <class... TARGET> struct Query : public base_query {

	template <class T, size_t ... A> T make(sqlite3_stmt *s, std::index_sequence<A...>) {
		return T { stepper<TARGET>(s, A)... };
	}

	template <class... A> Query(sqlite3 *db, const std::string &query, const A& ... args) : base_query { db, query, args... } {}

	template <class T> T get() {
		if(lastCode < 0)
			lastCode = sqlite3_step(stmt);

		if(lastCode == SQLITE_ROW) {
			return make<T>(stmt, std::make_index_sequence<sizeof...(TARGET)>());
		} else
			throw db_exception("No more rows");
	}

	std::tuple<TARGET...> get_tuple() {
		if(lastCode < 0)
			lastCode = sqlite3_step(stmt);
		if(lastCode == SQLITE_ROW) {
			return make<std::tuple<TARGET...>>(stmt, std::make_index_sequence<sizeof...(TARGET)>());
		} else
			throw db_exception("No more rows");
	}
};

template <class T> struct Query<T> : public base_query {
	template <class... A> Query(sqlite3 *db, const std::string &query, const A& ... args) : base_query { db, query, args... } {}

	T get() {
		if(lastCode < 0)
			lastCode = sqlite3_step(stmt);

		if(lastCode == SQLITE_ROW) {
			return stepper<T>(stmt, 0);
		} else
			throw db_exception("No more rows");
	}
};

public:
	Database(const std::string &file = "", int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE) : db(nullptr) {
		if(file != "") {
			int rc = sqlite3_open_v2(file.c_str(), &db, flags, nullptr);
			if(rc != SQLITE_OK)
				throw db_exception("Could not open database");
		}
	}

	~Database() {
		if(db)
			sqlite3_close(db);
	}

	template <class... TARGET, class... A> Query<TARGET...> query(const std::string &q, const A& ... args) const {
		return Query<TARGET...>(db, q, args...);
	}

	template <class... TARGET, class A> Query<TARGET...> query(const std::string &q, const std::vector<A> &args) const {
		return Query<TARGET...>(db, q, args);
	}

	template <class... A> void exec(const std::string &q, const A& ... args) const {
		query(q, args...).step();
	}

	uint64_t last_rowid() {
		return sqlite3_last_insert_rowid(db);
	}

	bool has_table(const std::string &table) {
		auto q = query<std::string>("SELECT name FROM sqlite_master WHERE type='table' AND name=?", table);
		return q.step();
	}

	struct Transaction {
		struct Internal {
			Internal(sqlite3 *db) : db(db) {
				char *err;
				int rc = sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, &err);
				if(rc != SQLITE_OK)
					throw db_exception(err);
			}
			~Internal() {
				if(db) {
					if(std::uncaught_exception())
						rollback();
					else
						commit();
				}
			}
			void commit() {
				char *err;
				int rc = sqlite3_exec(db, "COMMIT", nullptr, nullptr, &err);
				if(rc != SQLITE_OK)
					throw db_exception(err);
				db = nullptr;
			}
			void rollback() {
				char *err;
				int rc = sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, &err);
				if(rc != SQLITE_OK)
					throw db_exception(err);
				db = nullptr;
			}
			sqlite3 *db;
		};
		Transaction(sqlite3 *db) : i { std::make_shared<Internal>(db) } {}
		void commit() {
			i->commit();
		}
		void rollback() {
			i->rollback();
		}
	private:
		std::shared_ptr<Internal> i;
	};

	Transaction transaction() {
		return Transaction(db);
	}

private:
	sqlite3 *db;
};

}

#endif // SQLITE_DATABASE_H