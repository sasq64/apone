
#include "net.h"

#include "asio/asio.hpp"

#include <coreutils/utils.h>
#include <coreutils/log.h>

#include <vector>
#include <functional>
#include <cstdint>

using asio::ip::tcp;
using namespace std;
using namespace utils;

namespace net {

asio::io_service Connection::global_io_service;

void Connection::connect(const string &target, int port) {
	string service;
	string server = target;

	if(port == -1) {
		auto parts = utils::split(server, ":");
		if(parts.size() > 1) {
			service = parts[parts.size()-1];
			server = parts[0];
		}
	} else
		service = to_string(port);
	LOGD("Connecting %s %s\n", server, service);
	tcp::resolver::query query(server, service);
	auto endpoints = resolver.resolve(query);
	asio::connect(socket, endpoints);
}

string Connection::getLine() {
	asio::read_until(socket, data, '\n');
	istream is(&data);
	char tmp[16384];
	is.getline(tmp, sizeof(tmp));
	char *ptr = tmp;
	while(*ptr != 10 && *ptr != 13)
		ptr++;
	*ptr = 0;
	return string(tmp);
}

void Connection::read_lines() {
	//vector<uint8_t> data(128*1024);
	//data.prepare(128*1024);
	asio::async_read_until(socket, data, "\r\n\r\n", [&](const asio::error_code &e, size_t n) {
		LOGD("Got %d chars", n);
		istream is(&data);
		while(!is.eof()) {
			string line;
			getline(is, line);
			LOGD("%s", line);
		}
	});
	LOGD("Async started");
}

/*
class Server {
public:
	Server(int port) {
	}

	void run() {
		while(true) {
			asio::io_service io_service;
			tcp::socket socket(io_service);
			tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 12345));
			LOGD("Listening\n");
			acceptor.accept(socket);
			LOGD("Got connection\n");

			thread t(connect_cb, Connection(socket));
			t.detach();

		}
	}

	void onConnect(function<void(Connection)> f) {
		connect_cb = f;
	}

	function<void(Connection)> connect_cb;
};
*/

} // namespace net
