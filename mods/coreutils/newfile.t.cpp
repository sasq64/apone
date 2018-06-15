
#include "newfile.h"
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

using namespace apone;

TEST_CASE("basic", "[file]")
{
    File{ "dummy", File::Write }.write<uint32_t>(0x12345678);
    REQUIRE(File{ "dummy" }.read<uint32_t>() == 0x12345678);
    fs::remove("dummy");
}

TEST_CASE("fp interop", "[file]")
{
    File f{ "dummy", File::Write };
    fwrite("testing", 1, 8, f);
    f.writeString("write");
    f.close();

    FILE* fp = fopen("dummy", "rb");
    File f2{ fp };
    auto s = f2.readString();
    REQUIRE(s == "testing");
    char buffer[16];
    REQUIRE(fread(buffer, 1, sizeof(buffer), fp) == 6);
    REQUIRE(strcmp(buffer, "write") == 0);
    f2.close();

    fs::remove("dummy");


}
TEST_CASE("listFiles", "[file]")
{

}
