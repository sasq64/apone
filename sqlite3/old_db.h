#ifndef SQLITE_DATABASE_H
#define SQLITE_DATABASE_H

#include "sqlite3.h"
#include <coreutils/log.h>
#include <mutex>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <map>

#define BACKWARD_HAS_BFD 1
#include "../../bbs/backward-cpp/backward.hpp"

namespace sqlite3db {

class db_exception : public std::exception {
public:
	db_exception(const char *ptr = "DB Exception") : msg(ptr) {
		st.load_here(32);
		backward::Printer p; p.print(st);

	}
	virtual const char *what() const throw() { return msg; }
private:
	const char *msg;
	backward::StackTrace st;
};

struct Statement {
	Statement(sqlite3_stmt *s, int p) : stmt(s), pos(p) {}
	sqlite3_stmt *stmt;
	int pos;
};

//template <class T> int bindArg(Statement &s, typename std::enable_if<std::is_integral<T>::value, long>::type arg)  {
//template <class T> int bindArg(Statement &s, typename std::enable_if<std::is_integral<T>::value, long>::type arg)  {
int bindArg(Statement &s, uint64_t arg);

template <class T> int bindArg(Statement &s, typename std::enable_if<std::is_floating_point<T>::value, double>::type arg)  {
	return sqlite3_bind_double(s.stmt, s.pos++, arg);
}

int bindArg(Statement &s, const char *arg);

int bindArg(Statement &s, const std::string &arg);

template <typename T> int bindArg(Statement &s, const std::vector<T> &arg)  {
	return sqlite3_bind_blob(s.stmt, s.pos++, &arg[0], arg.size() * sizeof(T), nullptr);
}
//int bindArg(uint64_t arg)  {
//	//sqlite3_bind_int64(s.stmt, s.pos++, arg);
//	return 0;
//}

//template <class T> typename std::enable_if<std::is_floating_point<T>::value, double>::type bindArg(T arg) { return arg; }
//template <class T> typename std::enable_if<std::is_pointer<T>::value, T>::type bindArg(T arg) { return arg; }
//const char* bindArg(const std::string& arg) { return arg.c_str(); }


template <typename T> T stepper(Statement &s) {
	throw db_exception("Uknown target type");
}

template <> uint64_t stepper(Statement &s);
template <> int stepper(Statement &s);
template <> double stepper(Statement &s);
template <> const char * stepper(Statement &s);
template <> std::string stepper(Statement &s);
template <> std::vector<uint8_t> stepper(Statement &s);
template <> std::vector<uint64_t> stepper(Statement &s);



class Database {

template <class... Target> struct Query {
	template <class... A> Query(sqlite3 *db, const std::string &query, const A& ... args) : db(db), stmt(nullptr), lastCode(-1) {
		//sqlite3_stmt *stmt = nullptr;
		int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		LOGD("CONSTRUCT stmt %p", stmt);	
		//rc = sqlite3_bind_blob(stmt, 1, buffer, size, SQLITE_STATIC);

		Statement s { stmt, 1 };
		std::vector<int> results = { bindArg(s, args)... };

		for(int &r : results) {
			if(r != SQLITE_OK)
				throw db_exception(sqlite3_errmsg(db));
		}
		//lastCode = sqlite3_step(s.stmt);
		//if(rc != SQLITE_OK)
		//	throw db_exception(sqlite3_errmsg(db));
	};

	Query& operator=(const Query &q) = delete;
	Query(const Query &q) = delete;

	Query(Query &&q) {
		LOGD("Copy cons");
		db = q.db;
		stmt = q.stmt;
		lastCode = q.lastCode;
		q.stmt = nullptr;
	}

	Query& operator=(Query &&q) {

		finalize();

		LOGD("Assign %p", q.stmt);
		db = q.db;
		stmt = q.stmt;
		lastCode = q.lastCode;
		q.stmt = nullptr;
		return *this;
	};

	~Query() {
		LOGD("Destroy %p", stmt);
		finalize();
	}

	//bool done() {
	//	return (lastCode != SQLITE_ROW);
	//}

	void finalize() {
		LOGD("finalize %p", stmt);	
		if(stmt)
			sqlite3_finalize(stmt);
		stmt = nullptr;
	}

	bool step() {
		lastCode = sqlite3_step(stmt);
		return (lastCode == SQLITE_ROW);
	}

/*
	void step() {
		Statement s { stmt, 0 };
		if(lastCode == SQLITE_ROW) {
			lastCode = sqlite3_step(s.stmt);
		} else
			throw db_exception("No more rows");
	}

	template <class B> B step() {
		B target;
		Statement s { stmt, 0 };
		if(lastCode == SQLITE_ROW) {
			target = stepper<B>(s);
			lastCode = sqlite3_step(s.stmt);
		} else
			throw db_exception("No more rows");
		return target;
	}

	template <class A, class B, class ...C> std::tuple<A, B, C...> step() {

		LOGD("STEP stmt %p", stmt);	
		std::tuple<A, B, C...> target;
		Statement s { stmt, 0 };
		if(lastCode == SQLITE_ROW) {
			target = std::make_tuple(stepper<A>(s), stepper<B>(s), stepper<C>(s)...);
			lastCode = sqlite3_step(s.stmt);
		} else
			throw db_exception("No more rows");
		return target;
	}
*/
	template <class T> T get() {
		Statement s { stmt, 0 };
		if(lastCode < 0)
			lastCode = sqlite3_step(stmt);

		if(lastCode == SQLITE_ROW) {
			return T(stepper<Target>(s)...);
		} else
			throw db_exception("No more rows");
	}


/*
	template <class ...A, class T> T stepw() {
		Statement s { stmt, 0 };
		if(lastCode == SQLITE_ROW) {
			target = T(stepper<A>(s)...);
			lastCode = sqlite3_step(s.stmt);
		} else
			throw db_exception("No more rows");
		return target;
	}
*/
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int lastCode;
};

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
				if(!data[i])
					v.push_back("");
				else
					v.push_back(data[i]);
			}
			auto cb = (Callback*)t;
			cb->func(cb->index, v);
			return 0;
		}
	};

	template <class...Target, class... A> Query<Target...> query(const std::string &query, const A& ... args) {
		return Query<Target...>(db, query, args...);
	}

	template <class... A, class ... B> void exec2(std::tuple<B...> &target, const std::string &query, const A& ... args) {
		sqlite3_stmt *stmt = NULL;
		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		//rc = sqlite3_bind_blob(stmt, 1, buffer, size, SQLITE_STATIC);

		Statement s { stmt, 1 };

		std::vector<int> results = { bindArg(s, args)... };

		for(int &r : results) {
			if(r != SQLITE_OK)
				throw db_exception(sqlite3_errmsg(db));
		}

		while(true) {
			rc = sqlite3_step(stmt);
			//if(rc != SQLITE_OK)
			//	throw db_exception(sqlite3_errmsg(db));
			LOGD("rc %d", rc);
			if(rc == SQLITE_ROW) {
				s.pos = 1;
				target = std::make_tuple(stepper<B>(s)...);
			} else
				break;
		}
		sqlite3_finalize(stmt);
	}

	template <class... A, class ... B> void exec2(std::tuple<B...> &target, const std::string &query, std::function<void()> f, const A& ... args) {
		sqlite3_stmt *stmt = NULL;
		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		//rc = sqlite3_bind_blob(stmt, 1, buffer, size, SQLITE_STATIC);

		Statement s { stmt, 1 };

		std::vector<int> results = { bindArg(s, args)... };

		for(int &r : results) {
			if(r != SQLITE_OK)
				throw db_exception(sqlite3_errmsg(db));
		}

		while(true) {
			rc = sqlite3_step(stmt);
			//if(rc != SQLITE_OK)
			//	throw db_exception(sqlite3_errmsg(db));
			LOGD("rc %d", rc);
			if(rc == SQLITE_ROW) {
				s.pos = 1;
				target = std::make_tuple(stepper<B>(s)...);
				f();
			} else
				break;
		}
		sqlite3_finalize(stmt);
	}

	template <class... A, class B> void exec2(B &target, const std::string &query, std::function<void()> f, const A& ... args) {
		sqlite3_stmt *stmt = NULL;
		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		//rc = sqlite3_bind_blob(stmt, 1, buffer, size, SQLITE_STATIC);

		Statement s { stmt, 1 };

		std::vector<int> results = { bindArg(s, args)... };

		for(int &r : results) {
			if(r != SQLITE_OK)
				throw db_exception(sqlite3_errmsg(db));
		}

		while(true) {
			rc = sqlite3_step(stmt);
			//if(rc != SQLITE_OK)
			//	throw db_exception(sqlite3_errmsg(db));
			LOGD("rc %d", rc);
			if(rc == SQLITE_ROW) {
				s.pos = 1;
				//target = std::make_tuple(stepper<B>(s)...);
				target = stepper<B>(s);
				f();
			} else
				break;
		}
		sqlite3_finalize(stmt);
	}

	template <class... A> void execf(const std::string &query, std::function<void(int i, const std::vector<std::string> &result)> f, const A& ... args) {
		char *err;
		char *q = sqlite3_mprintf(query.c_str(), normalizeArg(args)...);
		Callback cb {f};

		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_exec(db, q, Callback::callback, &cb, &err);
		sqlite3_free(q);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	void execf(const std::string &query, std::function<void(int i, const std::vector<std::string> &result)> f) {
		char *err;
		Callback cb {f};

		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_exec(db, query.c_str(), Callback::callback, &cb, &err);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	template <class... A> void exec(const std::string &query, const A& ... args) {
		char *err;
		char *q = sqlite3_mprintf(query.c_str(), normalizeArg(args)...);

		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_exec(db, q, nullptr, nullptr, &err);
		sqlite3_free(q);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}
//sqlite3_column_blob
//"INSERT INTO ONE(ID, NAME, LABEL, GRP, FILE) VALUES(NULL, 'fedfsdfss', NULL, NULL, ?)"
	template <typename T> void get_blob(const std::string &query, int col, std::vector<T> &data) {
		sqlite3_stmt *stmt = NULL;

		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		//rc = sqlite3_bind_blob(stmt, 1, buffer, size, SQLITE_STATIC);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		rc = sqlite3_step(stmt);
		//if(rc != SQLITE_OK)
		//	throw db_exception(sqlite3_errmsg(db));
		LOGD("rc %d", rc);
		if(rc == SQLITE_ROW) {
			const void *p = sqlite3_column_blob(stmt, col);
			int size = sqlite3_column_bytes(stmt, col);
			//if(rc != SQLITE_OK)
			//	throw db_exception(sqlite3_errmsg(db));
			data.resize(size / sizeof(T));
			memcpy(&data[0], p, size);
		}
		
		sqlite3_finalize(stmt);
	}

	template <typename T> void put_blob(const std::string &query, int col, const std::vector<T> &data) {
		sqlite3_stmt *stmt = NULL;

		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		rc = sqlite3_bind_blob(stmt, col, &data[0], data.size() * sizeof(T), SQLITE_STATIC);
		if(rc != SQLITE_OK)
			throw db_exception(sqlite3_errmsg(db));
		rc = sqlite3_step(stmt);
		LOGD("STEP %d with size %d bytes", rc, data.size() * sizeof(T));
		//if(rc != SQLITE_OK)
		//	throw db_exception(sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
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

		std::lock_guard<std::mutex> guard(lock);
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

		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_exec(db, q, Callback::callback, &cb, &err);
		sqlite3_free(q);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	void exec(const std::string &query) {
		char *err;

		std::lock_guard<std::mutex> guard(lock);
		int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err);
		if(rc != SQLITE_OK)
			throw db_exception(err);
	}

	uint64_t last_rowid() {
		std::lock_guard<std::mutex> guard(lock);
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
	std::mutex lock;
};

}

#endif // SQLITE_DATABASE_H