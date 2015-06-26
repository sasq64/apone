#ifndef NET_H
#define NET_H

#include "asio/asio.hpp"

#include <string>

namespace net {

/* Represents a socket connection. Not copyable */
class Connection {
public:

	Connection() : socket(global_io_service), resolver(global_io_service) {}
	Connection(const Connection &) = delete;
	//Connection(const Connection &c) : socket(c.socket), resolver(io_service) {
	//}
	Connection(Connection &&c) : socket(std::move(c.socket)), resolver(global_io_service) {}
	Connection(asio::ip::tcp::socket &socket) : socket(std::move(socket)), resolver(global_io_service) {}

	Connection(const std::string &server, int port = -1) : socket(global_io_service), resolver(global_io_service) {
		connect(server, port);
	}

	void connect(const std::string &target, int port = -1);

/*
	template <typename ... T> size_t read(T ... target) {
		return socket.read_some(asio::buffer(target...));
	}
*/

	// NOTE: Does not deal with short writes
	void write(const std::string &s) {
		socket.send(asio::buffer(s));
	}

	// NOTE: Does not deal with short writes
	template <typename ... T> void write(T ... target) {
		socket.send(asio::buffer(target...));
	}

	static void run() {
		//LOGD("RUN");
		global_io_service.run();
		global_io_service.reset();
	}

	std::string getLine();

	void read_lines();

	asio::ip::tcp::socket& getSocket() { return socket; }

private:

	static asio::io_service global_io_service;

	asio::streambuf data;	
	asio::ip::tcp::socket socket;
	asio::ip::tcp::resolver resolver;
};

};

#endif // NET_H
