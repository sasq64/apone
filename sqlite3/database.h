#ifndef SQLITE_DATABASE_H
#define SQLITE_DATABASE_H

#include "sqlite3.h"

namespace sqlite3db {

class db_exception : public std::exception {
public:
	db_exception(const char *ptr = "DB Exception") : msg(ptr) {}
	virtual const char *what() const throw() { return msg; }
private:
	const char *msg;
};

class Database {
public:
	Database(const std::string &file, int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE) {
		int rc = sqlite3_open_v2(file.c_str(), &db, flags, nullptr);
		if(rc != SQLITE_OK)
			throw db_exception("Could not open database");
	}

	template <class T> typename std::enable_if<std::is_integral<T>::value, long>::type normalizeArg(T arg)  { return arg; }
	template <class T> typename std::enable_if<std::is_floating_point<T>::value, double>::type normalizeArg(T arg) { return arg; }
	template <class T> typename std::enable_if<std::is_pointer<T>::value, T>::type normalizeArg(T arg) { return arg; }
	const char* normalizeArg(const std::string& arg) { return arg.c_str(); }

	struct Callback {
		Callback(std::function<void(int i, const std::vector<std::string> &result)> f) : func(f) {}
		std::function<void(int i, const std::vector<std::string> &result)> func;
		int index;
		static int callback(void *t, int n, char **data, char **names) {
			std::vector<std::string> v;
			for(int i=0; i<n; i++) {
				v.push_back(data[i]);
			}
			auto cb = (Callback*)t;
			cb->func(cb->index, v);
			return 0;
		}
	};

	template <class... A> void execf(const std::string &query, std::function<void(int i, const std::vector<std::string> &result)> f, const A& ... args) {
		char *err;
		char *q = sqlite3_mprintf(query.c_str(), normalizeArg(args)...);
		Callback cb {f};
		int rc = sqlite3_exec(db, q, Callback::callback, &cb, &err);
		sqlite3_free(q);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	void execf(const std::string &query, std::function<void(int i, const std::vector<std::string> &result)> f) {
		char *err;
		Callback cb {f};
		int rc = sqlite3_exec(db, query.c_str(), Callback::callback, &cb, &err);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	template <class... A> void exec(const std::string &query, const A& ... args) {
		char *err;
		char *q = sqlite3_mprintf(query.c_str(), normalizeArg(args)...);
		int rc = sqlite3_exec(db, q, nullptr, nullptr, &err);
		sqlite3_free(q);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	//template <class T, class A> T convertArg(const A &a) {
	//}

	template <class T> typename std::enable_if<std::is_integral<T>::value, long>::type convertArg(const std::vector<std::string> &a) {
		return std::stol(a[0]);
	} 

	template <class S, class T> std::pair<S, T> convertArg(const std::vector<std::string> &a) {
		return std::make_pair(std::stol(a[0]), std::stol(a[1]));
	} 

	long convertArg(long a) {
		return a;
	} 

	template <class T, class... A> bool select_first(T &target, const std::string &query, const A& ... args) {
		char *err;
		bool found = false;
		char *q = sqlite3_mprintf(query.c_str(), normalizeArg(args)...);
		Callback cb {[&](int i, const std::vector<std::string> &result) {
			target = convertArg<T>(result);
			found = true;
		}};
		int rc = sqlite3_exec(db, q, Callback::callback, &cb, &err);
		sqlite3_free(q);
		if(rc != SQLITE_OK)
			throw db_exception(err);
		return found;
	}

	template <template <typename, typename> class Container, class V, class A, class...S>
	void select(const std::string &query, Container<V, A> const& target, const S& ... args) {
		char *err;
		char *q = sqlite3_mprintf(query.c_str(), normalizeArg(args)...);
		Callback cb {[&](int i, const std::vector<std::string> &result) {
			target.emplace_back(result);
		}};
		int rc = sqlite3_exec(db, q, Callback::callback, &cb, &err);
		sqlite3_free(q);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	void exec(const std::string &query) {
		char *err;
		int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	uint64_t last_rowid() {
		return sqlite3_last_insert_rowid(db);
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