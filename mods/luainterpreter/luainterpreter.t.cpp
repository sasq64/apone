#include "luainterpreter.h"

#include <catch.hpp>

TEST_CASE("utils::luainterpreter", "interpreter")
{
    LuaInterpreter lua;

    lua.registerFunction("testFunc", [](std::string s, int x, float f) -> int {
        return 1 + x + f;
    });

    const std::string luaCode = R"(
	function test (a, b)
		testFunc('arne', 199)
		x = testFunc('hello', 2, 3)
		return x * 2 + a + b
	end
	)";

    lua.load(luaCode);
    auto res = lua.call<float>("test", 4, 2);
    REQUIRE(res == 18);
}
