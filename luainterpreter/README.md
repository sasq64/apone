
The LuaInterpreter allows you to easily register functions that are callable from lua, as well as call lua functions
directly from C++, without messing with the lua stack yourself;

```C++
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
