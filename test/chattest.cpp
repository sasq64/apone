
#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <bbsutils/telnetserver.h>
#include <bbsutils/console.h>
#include <bbsutils/editor.h>

#include <string>
#include <algorithm>

using namespace std;
using namespace bbs;
using namespace utils;

int main(int argc, char **argv) {

	setvbuf(stdout, NULL, _IONBF, 0);
	logging::setLevel(logging::DEBUG);

	// Turn off logging from the utility classes
	logging::useLogSpace("utils", false);

	vector<string> chatLines;
	mutex chatLock;

	auto addChatLine = [&](const string &line) {
		lock_guard<mutex> guard(chatLock);
		chatLines.push_back(line);
	};

	TelnetServer telnet { 12345 };
	telnet.setOnConnect([&](TelnetServer::Session &session) {
		string userName;
		try {
			session.echo(false);
			auto termType = session.getTermType();		
			unique_ptr<Console> con;

			// We just assume that no terminal reported means petscii.
			if(termType.length() > 0) {
				con = make_unique<AnsiConsole>(session);
			} else {
				con = make_unique<PetsciiConsole>(session);
			}
			Console &console = *con;

			console.flush();
			auto h = console.getHeight();
			auto w = console.getWidth();
			LOGD("New connection, terminal:'%s', size:%dx%d", termType, w, h);

			console.write("\nNAME:");
			userName = console.getLine();
			addChatLine(userName + " joined");

			uint lastLine = 0;
			auto ypos = 0;
			console.clear();
			console.fill(Console::BLUE, 0, -2, 0, 1);
			console.put(-13, -2, "F7 = Log out", Console::CURRENT_COLOR, Console::BLUE);
			console.moveCursor(0, -1);
			
			// Don't show more lines from the backlog than fits the screen
			{ lock_guard<mutex> guard(chatLock);
				if((int)chatLines.size() > h)
					lastLine = chatLines.size() - h;
			}

			auto lineEd = make_unique<LineEditor>(console);

			while(true) {

				auto key = lineEd->update(500);				
				switch(key) {
				case 0:
					addChatLine(userName + ": " + lineEd->getResult());
					console.fill(Console::BLACK, 0, -1, 0, 1);
					console.moveCursor(0, -1);
					lineEd = make_unique<LineEditor>(console);
					break;
				case Console::KEY_F7:
					addChatLine(userName + " logged out");
					session.close();
					return;
				}

				// Update screen with new chat lines if necessary
				{ lock_guard<mutex> guard(chatLock);
					auto newLines = false;
					while(chatLines.size() > lastLine) {
						newLines = true;
						auto msg = chatLines[lastLine++];
						ypos++;
						if(ypos > h-2) {
							console.scrollScreen(1);
							console.fill(Console::BLACK, 0, -3, 0, 1);
							console.fill(Console::BLUE, 0, -2, 0, 1);
							console.put(-13, -2, "F7 = Log out", Console::CURRENT_COLOR, Console::BLUE);
							ypos--;
						}
						console.put(0, ypos-1, msg);

					}
					if(newLines)
						lineEd->refresh();

					console.flush();
				}
			}
		} catch (TelnetServer::disconnect_excpetion e) {
			LOGD("Client disconnected");
			addChatLine(userName + " disconnected");
		}
	});
	telnet.run();

	return 0;
}
