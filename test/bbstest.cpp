#include <coreutils/utils.h>
#include <bbsutils/telnetserver.h>

using namespace bbs;
using namespace utils;

int main() {
 
	TelnetServer telnet { 12345 };
	telnet.setOnConnect([&](TelnetServer::Session &session) {
		session.write("name:");
		auto name = session.getLine();
		session.write(format("goodbye %s\r\n", name));
		session.close();
	});
	telnet.run();
	return 0;
}
