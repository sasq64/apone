
This is a small, modern C++ interface to sqlite3, on top of the old C api


BASIC USAGE
-----------

Create a database from a filename. Will be created if it does not exist.

	Database db { "datafile.db" };

Use `exec()` for commands that you do not need results from. Parameter types must match database types or
an exceptiion will be thrown.

	db.exec("CREATE TABLE IF NOT EXISTS user (name TEXT, id INT, image BLOB)");

	string userName = "james";
	uint64_t id = 123;

	db.exec("INSERT INTO user (name, id) VALUES (?, ?)", name, id);
	db.exec("INSERT INTO user (name, id) VALUES (?, ?)", id, name); // <-- Will throw an exception

Use the template method `query()` when you want to read results. You need to provide type arguments for the selected fields.

	auto q = db.query<uint64_t, string>("SELECT id,name FROM user");

This is again typesafe, an exception will be thrown if the C++ types does not match
the database types.

	auto q = db.query<string, string>("SELECT id,name FROM user");
	auto q.step(); // Will throw exception

Use `step()` and `get()` to fetch rows from the result. `get()` is a template function that
will create an object of the given class using the row result. The constructor of the
object must match the types defined in the query.

	struct User {
		User(uint64_t id, string name) : id(id), name(name) {}
		uint64_t id;
		string name;
		std::vector<uint8_t> pixels;
	};

	std::vector<User> users;
	while(q.step()) {
		users.push_back(q.get<User>());
	}

Blobs are supported using `std::vector`.

	std::vector<uint8_t> pixels;
	db.exec("INSERT INTO user (image) VALUES (?)", pixels);

Note that if you only select one field, `get()` can be called directly and returns that type.

	auto q = db.query<std::vector<uint8_t>>("SELECT image FROM user WHERE id=?", id);
	q.step();
	auto pixels2 = q.get(); // We don't need get<vector<uint8_t>>() here;

If you haven't called `step()` on a query, it will be called automatically by `get()`

	uint64_t id = db.query<uint64_t>("SELECT id FROM user WHERE name=?", userName).get();
