
#include "net.h"

#include "asio/asio.hpp"

#include <coreutils/utils.h>
#include <coreutils/log.h>
#include <coreutils/file.h>

#include <vector>
#include <functional>
#include <cstdint>

using asio::ip::tcp;
using namespace std;
using namespace utils;

static asio::io_service global_io_service;

namespace net {

/* Represents a socket connection. Not copyable */
class Connection {
public:

	Connection() : socket(io_service), resolver(io_service) {}
	Connection(const Connection &) = delete;
	Connection(Connection &&c) : io_service(c.socket.get_io_service()), socket(std::move(c.socket)), resolver(io_service) {}
	Connection(tcp::socket &socket) : socket(std::move(socket)), resolver(io_service) {}

	Connection(const std::string &server, int port = -1) : socket(io_service), resolver(socket.get_io_service()) {
		connect(server, port);
	}

	void connect(const std::string &target, int port = -1) {
		std::string service;
		std::string server = target;

		if(port == -1) {
			auto parts = utils::split(server, ":");
			if(parts.size() > 1) {
				service = parts[parts.size()-1];
				server = parts[0];
			}
		} else
			service = std::to_string(port);
		LOGD("Connecting %s %s\n", server, service);
		tcp::resolver::query query(server, service);
		auto endpoints = resolver.resolve(query);
    	asio::connect(socket, endpoints);
	}

	template <typename ... T> size_t read(T ... target) {
		return socket.read_some(asio::buffer(target...));
	}

	template void write(const std::string &s) {
		socket.send(asio::buffer(s));
	}

	template <typename ... T> void write(T ... target) {
		socket.send(asio::buffer(target...));
	}

	void run() {
		io_service.run();
	}



	std::string getLine() {
		asio::streambuf line;
		asio::read_until(socket, line, '\n');
		std::istream is(&line);
		char tmp[16384];
		is.getline(tmp, sizeof(tmp));
		char *ptr = tmp;
		while(*ptr != 10 && *ptr != 13)
			ptr++;
		*ptr = 0;
		return std::string(tmp);
	}

	void read_lines() {
		//std::vector<uint8_t> data(128*1024);
		//data.prepare(128*1024);
		asio::async_read_until(socket, data, "\r\n\r\n", [&](const asio::error_code &e, size_t n) {
			LOGD("Got %d chars", n);
			std::istream is(&data);
			while(!is.eof()) {
	    		std::string line;
	    		std::getline(is, line);
	    		LOGD("%s", line);
	    	}
    	});
    	LOGD("Async started");
	}

	tcp::socket& getSocket() { return socket; }

private:
	asio::streambuf data;

	asio::io_service &io_service = global_io_service;
    tcp::socket socket;
    tcp::resolver resolver;
};

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

			std::thread t(connect_cb, Connection(socket));
			t.detach();

		}
	}

	void onConnect(std::function<void(Connection)> f) {
		connect_cb = f;
	}

	std::function<void(Connection)> connect_cb;
};


void WebGetter::getFile(const string &url, function<void(const File&)> callback) {
	int hostStart = 0;
	if(startsWith(url, "http://"))
		hostStart = 7;
	int firstSlash = url.find_first_of('/', hostStart);
	string host = url.substr(hostStart,firstSlash-hostStart);
	string path = url.substr(firstSlash);
	string req = utils::format("GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);

	Connection c(host, 80);
    c.write(req);
    LOGD("Wrote '%s' to %s", req, path);

    asio::streambuf data;
	asio::read_until(c.getSocket(), data, "\r\n\r\n");
	std::istream is(&data);
	while(!is.eof()) {
		std::string line;
		std::getline(is, line);
		LOGD("%s", line);
	}
}

/*
int http_get(unordered_map<string, string> &header) {
	auto l = c.read(&data[0], data.size());
	string txt((const char*)&data[0], l);
	printf("Got %lu bytes\n%s\n", l, txt.c_str());
	auto lines = utils::split(txt, "\r\n");
	bool first = true;
	int code = 0;
	for(const auto &ln : lines) {
		if(first) {
			auto p = utils::split(ln, " ");
			code = atoi(p[1]);
			first = false;
		} else {
			auto p = utils::split(ln, ": ");
			LOGD("'%s' = '%s'\n", p[0], p[1]);
			header[p[0]] = p[1];
	    }
	}
}
*/
void net_test() {
	using namespace std;

	Server s(12345);
	s.onConnect([](Connection c) {
	    while(true) {
		    auto line = c.getLine();
		    c.write(utils::format("Hello '%s'\n", line));
		}
	});
	s.run();

	while(true) {
		utils::sleepms(500);
	}



string req = utils::format(
"GET %s HTTP/1.1\r\n"
"Host: %s\r\n"
//"Connection: close\r\n"
//"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
//"User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.135 Safari/537.36\r\n"
//"Accept-Encoding: gzip, deflate, sdch\r\n"
//"Accept-Language: en-US,en;q=0.8,sv;q=0.6\r\n"
"\r\n", "/release/download.php?id=81345", "csdb.dk");

    vector<uint8_t> data(3000);
    Connection c("csdb.dk:80");
    printf("--%s--", req.c_str());
    c.write(req);
    LOGD("Reading\n");

#if 1
    c.read_lines();
    c.run();
    while(true) {
    	LOGD("WAITING");
    	utils::sleepms(1000);
    }
#else
    auto l = c.read(&data[0], data.size());
    string txt((const char*)&data[0], l);
    printf("Got %lu bytes\n%s\n", l, txt.c_str());
    auto lines = utils::split(txt, "\r\n");
    bool first = true;
    unordered_map<string, string> header;
    int code = 0;
    for(const auto &ln : lines) {
    	if(first) {
    		auto p = utils::split(ln, " ");
    		code = atoi(p[1].c_str());
    		first = false;
    	} else {
	    	auto p = utils::split(ln, ": ");
	    	LOGD("'%s' = '%s'\n", p[0], p[1]);
	    	header[p[0]] = p[1];
	    }
    }

    if(code == 303) {
    }
#endif


}

} // namespace net
