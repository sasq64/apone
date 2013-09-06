Console
=======

* Is a grid of *Tiles*
* Each Tile has a character, a foreground color and a background color.
* The console keeps track of the clients screen contents
* The console keeps track of the clients real cursor position
* A *refresh* means to send the necessary (and preferably minimal) set of characters and control codes to mirror the internal representation on to the client screen.

Example
-------

	auto *console = createLocalConsole();

	// Write commands are written to the terminal directly
	console->write("One line\nNext line");

	vector<int> colors { Console::RED, Console::WHITE, Console::BLUE };

	for(int y=0; y<10; y++) {
		console->setFG(colors[y % 3]);
		console->put(0,y, format("Line #%d", y));
	}

	// Other output commands require a flush
	console->flush();

	// A telnetserver listens for connections on a port and for each connection runs a function in its own thread.
	TelnetServer telnet { 12345 };
	telnet.setOnConnect([&](TelnetServer::Session &session) {
		session.echo(false);
		auto console = unique_ptr<Console>(new AnsiConsole { session });
		console->write("LOGIN:);
		auto name = console->getLine();
	}