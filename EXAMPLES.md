## APONE EXAMPLES


### GRAPHICS EXAMPLE

```c++
    screen.open(false);
    Color color(0xff00ffff);
    Tween::make().from(color, 0xffffffff).seconds(2.0).onComplete([]() {
        printf("TADA!\n")
    };
    screen.render_loop([=](uint32_t delta) mutable {
        screen.clear();
        screen.circle(100, 100, color);
        screen.flip();
    });
 ```

### LUA EXAMPLE
```c++
    LuaInterpreter lua;

    // Register a function callable from lua, with specific arguments
    lua.registerFunction<int, string, int, float>("testFunc", [=](string s, int x, float f) -> int {
        LOGD("Got '%s' and %f", s, f);
        return x+10;
    });

    // Load a lua function into the interpreter
    const string luaCode = R"(
    function test (a, b)
        x = testFunc('hello', 3, 2.0)
        return 2*x
    end
    )";
    lua.load(luaCode);

    // Try calling the lua function, which in turn calls the C++ function
    int res = lua.call<double>("test", 4, 2);

    // res should be 26 here
```

### WEB EXAMPLE
```c++
    WebRPC rpc("http://something-funny-123.appspot.com/");

    JSon json;
    json.add("uid", to_string(trackid));
    json.add("name", name);
    json.add("version", API_VERSION);
    auto sl = json.add_array("songs");
    for(const SongInfo &info : songs) {
        sl.add(info.path);
    }
    rpc.post("get_song_url", json.to_string(), [=](const string &result) {
        JSon json = JSon::parse(result);
        for(auto pl : json("urls")) {
            list.emplace_back(pl.to_string());
        }
    });
```

### DATABASE EXAMPLE
```c++
    struct User {
        User(uint64_t id, string name) : id(id), name(name) {}
        uint64_t id;
        string name;
    };

    Database db { "users.db" };
    db.exec("CREATE TABLE IF NOT EXISTS user (name TEXT, id INT)");

    string userName = "james";
    uint64_t id = 123;

    db.exec("INSERT INTO user (name, id) VALUES (?, ?)", name, id);

    std::vector<User> users;
    auto q = db.query<uint64_t, string>("SELECT id,name FROM user");
    while(q.step()) {
        users.push_back(q.get<User>());
    }
```

