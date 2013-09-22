
#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <bbsutils/telnetserver.h>
#include <bbsutils/console.h>
#include <bbsutils/ansiconsole.h>
#include <bbsutils/petsciiconsole.h>
#include <bbsutils/editor.h>

#include <string>
#include <algorithm>

using namespace std;
using namespace bbs;
using namespace utils;

static string makeSize(int bytes) {
	auto s = 0;
	for(s=0; bytes > 1024; bytes /= 1024, s++);
	return format("%d%s", bytes, vector<string>{ "B", "K", "M", "G", "T" }[s]);
}

void runShell(Console &console) {
	console.flush();
	console.write("System shell. Use 'exit' to quit\n\n");

	auto rootDir = File::cwd();
	auto rootLen = rootDir.getName().length();
	auto currentDir = rootDir;

	while(true) {

		auto d = currentDir.getName().substr(rootLen);
		if(d == "") d = "/";

		console.write(d + " # ");
		auto line = console.getLine();
		console.write("\n");

		auto parts = split(line);
		if(parts.size() < 1)
			continue;
		auto cmd = parts[0];

		if(cmd == "ls") {
			for(const auto &f : currentDir.listFiles()) {
				auto n = path_filename(f.getName());
				if(f.isDir())
					n += "/";
				console.write(format("%-32s %s\n", n, makeSize(f.getSize())));
			}
		} else if(cmd == "cd") {
			File newDir { currentDir, parts[1] };
			if(newDir.isChildOf(rootDir) && newDir.exists())
				currentDir = newDir;
		} else if(cmd == "cat") {
			File file { currentDir, parts[1] };
			string contents((char*)file.getPtr(), file.getSize());
			console.write(contents);
			console.write("\n");
		} else if(cmd == "ed") {
			auto saved = console.getTiles();
			auto xy = console.getCursor();
			File file { currentDir, parts[1] };
			string contents((char*)file.getPtr(), file.getSize());
			FullEditor ed(console);

			ed.setString(contents);
			while(true) {
				auto rc = ed.update(500);
				if(rc == Console::KEY_F1) {
					console.setTiles(saved);					
					console.flush();
					console.moveCursor(xy);
					break;
				}
			}
		} else if(cmd == "exit")
			return;
	}
}

int main(int argc, char **argv) {

	setvbuf(stdout, NULL, _IONBF, 0);
	logging::setLevel(logging::DEBUG);

	// Turn off logging from the utility classes
	logging::useLogSpace("utils", false);

	TelnetServer telnet { 12345 };
	telnet.setOnConnect([&](TelnetServer::Session &session) {
		try {
			session.echo(false);
			auto termType = session.getTermType();		
			unique_ptr<Console> console;
			// We just assume that no terminal reported means petscii.
			if(termType.length() > 0) {
				console = make_unique<AnsiConsole>(session);
			} else {
				console = make_unique<PetsciiConsole>(session);
			}
			auto size = console->getSize();
			LOGD("New connection, termtype: %s, size (%dx%d)", termType, size.x, size.y);

			runShell(*console);
		} catch (TelnetServer::disconnect_excpetion e) {
			LOGD("Client disconnected");
		}
	});
	telnet.run();

	return 0;
}
