
#include "webgetter.h"
#include "net.h"

#include "asio/asio.hpp"

#include <coreutils/utils.h>
#include <coreutils/log.h>
#include <coreutils/file.h>

#include <vector>
#include <functional>
#include <cstdint>
#include <unordered_map>

using asio::ip::tcp;
using namespace std;
using namespace utils;

namespace net {

struct HttpSession::State {

	int chunkSize = 1024;
	int total;
	bool oneShot = false;
	std::string url;
	std::string host;
	std::string path;
	int port;
	std::function<void(const asio::error_code &e, size_t n)> handler;
	Callback callback;
	//std::vector<uint8_t> buffer;
	//std::shared_ptr<utils::File> file;
	Connection c;
	asio::streambuf data;
	std::unordered_map<std::string, std::string> header;
	int code;
	int headerSize;
	int cntSize;
	HttpSession::NetState state = NONE;
	//asio::deadline_timer timer;

};

std::atomic<int> HttpSession::ongoingCalls;

HttpSession::HttpSession(const string &url) : impl(make_shared<State>()) {

	ongoingCalls++;

	int hostStart = 0;
	impl->port = 80;

	impl->url = url;

	LOGD("URL:%s", url);

	auto parts = split(url, "://");

	if(parts.size() == 2) {
		if(parts[0] != "http") {
			impl->state = ERROR;
			return;
		}
	}

	if(startsWith(url, "http://"))
		hostStart = 7;

	int firstSlash = url.find_first_of('/', hostStart);
	impl->host = url.substr(hostStart, firstSlash - hostStart);
	impl->path = url.substr(firstSlash);
	int lastColon = impl->host.find_last_of(':');
	if(lastColon != string::npos) {
		impl->port = stol(impl->host.substr(lastColon+1));
		impl->host = impl->host.substr(0, lastColon);
	}

	LOGD("CONNECT %s %d", impl->host, impl->port);
	impl->c.connect(impl->host, impl->port);
	sendRequest(impl->path, impl->host);
}

HttpSession::HttpSession(HttpSession &&h) : impl(std::move(h.impl)) {
	ongoingCalls++;
}

HttpSession::~HttpSession() {
	ongoingCalls--;
}

HttpSession& HttpSession::operator=(HttpSession &&h) {
	impl = std::move(h.impl);
	return *this;
}


void HttpSession::sendRequest(const string &path, const string &host) {
	string req = utils::format("GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", urlencode(path, ":\\?;+# "), host);
	LOGD("REQ %p:\n%s", this, req);
	LOGD("%d", impl->data.size());
	impl->c.write(req);
	impl->state = CONNECT;
	asio::async_read_until(impl->c.getSocket(), impl->data, "\r\n\r\n", [=](const asio::error_code &e, size_t n) {
		//LOGD("Read %d bytes", n);
		parseHeader();
		impl->state = HEADERS;
		//if(impl->code == 200)
		readContent();
		//else
		//	impl->state = ERROR;
	});
}

void HttpSession::parseHeader() {
	istream is(&impl->data);
	string line;
	getline(is, line);
	auto rc = split(line, " ");
	impl->code = stol(rc[1]);
	LOGD(">>%s", line);
	while(!is.eof()) {
		getline(is, line);
		if(line == "" || line == "\r")
			break;
		LOGD("%s", line);
		auto parts = split(line, ": ", 2);
		if(parts.size() == 2) {
			parts[1] = rstrip(rstrip(parts[1], '\n'), '\r');
			impl->header[parts[0]] = parts[1];
		}
	}
	auto cl = impl->header["Content-Length"];
	impl->cntSize = cl != "" ? stol(impl->header["Content-Length"]) : 0;
	if(isRedirect()) {
		LOGD("Should redirect to '%s'", impl->header["Location"]);
		//sleepms(500);
	}
}

int HttpSession::contentSize() const { return impl->cntSize; }

void HttpSession::stream(const Callback &cb) {
	impl->callback = cb;
}

void HttpSession::getData(const Callback &cb) {
	impl->oneShot = true;
	impl->callback = cb;
}

HttpSession::Callback HttpSession::getCallback() const {
	return impl->callback;
}

bool HttpSession::isRedirect() const { return impl->code == 303 || impl->code == 302 || impl->code == 307; }

int HttpSession::returnCode() const { return impl->code; }

int HttpSession::getPort() const { return impl->port; }
string HttpSession::getHost() const { return impl->host; }

bool HttpSession::gotHeaders() const { return impl->state == HEADERS || impl->state == DONE || impl->state == EMPTY; }
bool HttpSession::done() const { return impl->state == DONE || impl->state == EMPTY; }

std::string HttpSession::getUrl() const { return impl->url; }

void HttpSession::stop() { impl->state = DONE; }

HttpSession::NetState HttpSession::getState() { return impl->state; }

void HttpSession::readContent() {
	int size = impl->cntSize - impl->data.in_avail();
	if(size <= 0) {

		if(impl->data.in_avail() > 0) {
			LOGD("ALL IN BUFFER");
			vector<uint8_t> buffer(impl->data.size());
			auto bufs = impl->data.data();
			copy(asio::buffers_begin(bufs), asio::buffers_end(bufs), buffer.begin());
			impl->total = impl->data.size();
			impl->state = DONE;
			impl->callback(*this, buffer);
		} else {
			LOGD("EMPTY");
			impl->state = EMPTY;
			if(!isRedirect()) {
				vector<uint8_t> v;
				impl->callback(*this, v);
			}
		}
		return;
	}

	if(!impl->callback || impl->oneShot) {
		LOGD("ALL AT ONCE");
		asio::async_read(impl->c.getSocket(), impl->data, asio::transfer_exactly(size), [&](const asio::error_code &e, size_t n) {
			impl->state = DONE;
			if(impl->oneShot) {
				auto v8 = getContent();
				impl->callback(*this, v8);
			}
		});
		return;
	}

	//filled = data.in_avail();
	impl->total = 0;
	//buffer.reserve(chunkSize);

	impl->handler = [=](const asio::error_code &e, size_t n) {
		//asio::deadline_timer t(io, boost::posix_time::seconds(5));
		//t->async_wait(

		impl->data.commit(n);
		//LOGD("Read %d bytes, avail %d", n, impl->data.size());
		//n += filled;
		vector<uint8_t> buffer(impl->data.size());
		auto bufs = impl->data.data();
		copy(asio::buffers_begin(bufs), asio::buffers_end(bufs), buffer.begin());
		impl->total += impl->data.size();
		//LOGD("Total %d", impl->total);

		if(n == 0 || impl->total == impl->cntSize) {
			impl->state = DONE;
			LOGD("Done reading");
		}

		impl->callback(*this, buffer);

		if(impl->state == DONE) {
			LOGD("Request stop");
			return;
		}

		impl->data.consume(impl->data.size());
		auto mutableBuffer = impl->data.prepare(16384);

		impl->c.getSocket().async_read_some(asio::buffer(mutableBuffer), impl->handler);
	};

	auto mutableBuffer = impl->data.prepare(16384);
	impl->c.getSocket().async_read_some(asio::buffer(mutableBuffer), impl->handler);
}

vector<uint8_t> HttpSession::getContent() {
	auto sz = contentSize();
	vector<uint8_t> v8(sz);
	istream is(&impl->data);
	is.read((char*)&v8[0], sz);
	return v8;
}

string HttpSession::getHeader(const string &what) const {
	return impl->header[what];
}

void WebGetter::update() {

	lock_guard<mutex>{m};

	Connection::run();
	auto it = sessions.begin();
	while(it != sessions.end()) {
		auto s = *it;
		if(s->gotHeaders() && s->isRedirect()) {
			auto location = s->getHeader("Location");
			if(!startsWith(location, "http://")) {
				location = format("http://%s:%d%s", s->getHost(), s->getPort(), urlencode(location, ":\\?;+ "));
			}
			LOGD("Redirecting to '%s'", location);
			auto callback = s->getCallback();
			it = sessions.erase(it);
			sessions.emplace_back(make_shared<HttpSession>(location));
			sessions.back()->stream(callback);
			it = sessions.end();
			continue;
		}
		if(s->done()) {
			it = sessions.erase(it);
			LOGD("Session done");
		} else
			it++;
	}

}

WebGetter::WebGetter(const string &cacheDir) : cacheDir(cacheDir) {
	sessions.reserve(4);
	quit = false;
	workerThread = thread([&]() {
		while(!quit) {
			sleepms(50);
			update();
		}
	});
}

void WebGetter::getFile(const string &url, function<void(const File&)> callback) {

	string u = url;
	auto target = make_shared<File>(cacheDir + "/" + urlencode(url, ":/\\?;"));
	if(target->exists() && target->getSize() > 0) {
		callback(*target);
		return;
	}
	lock_guard<mutex>{m};
	sessions.emplace_back(make_shared<HttpSession>(url));
	if(sessions.back()->getState() == HttpSession::ERROR) {
		callback(File::NO_FILE);
		sessions.erase(sessions.end()-1);
		return;
	}
	sessions.back()->stream([=](HttpSession &session, const vector<uint8_t> &v) {
		target->write(v);
		if(session.done()) {
			auto finalUrl = session.getUrl();
			target->close();
			if(session.returnCode() != 200) {
				target->remove();
				callback(File::NO_FILE);
			} else
			if(finalUrl != url) {
				auto finalTarget = make_shared<File>(cacheDir + "/" + urlencode(finalUrl, ":/\\?;"));
				std::rename(target->getName().c_str(), finalTarget->getName().c_str());
				symlink(finalTarget->getName().c_str(), target->getName().c_str());
				callback(*finalTarget);
			} else
				callback(*target);
		}
			
	});
}

WebGetter::~WebGetter() {
	quit = true;
	workerThread.join();
}

void WebGetter::streamData(const string &url, function<bool(const uint8_t *data, int size)> callback) {
	lock_guard<mutex>{m};
	sessions.emplace_back(make_shared<HttpSession>(url));
	bool sizeReported = false;
	sessions.back()->stream([=](HttpSession &session, const vector<uint8_t> &v) mutable {		
		if(!sizeReported) {
			sizeReported = true;
			LOGD("SIZE:%d", session.contentSize());
			if(!callback(nullptr, session.contentSize())) {
				session.stop();
				return;
			}
		}
		if(!callback(&v[0], v.size()))
			session.stop();
		else
		if(session.done())
			callback(nullptr, 0);
	});
}

int WebGetter::inProgress() { 
	return HttpSession::ongoingCalls;
}

bool WebGetter::inCache(const std::string &url) const {
	auto target = File(cacheDir + "/" + urlencode(url, ":/\\?;"));
	return target.exists();
}

void WebGetter::getData(const std::string &url, std::function<void(const std::vector<uint8_t> &data)> cb) {
	lock_guard<mutex>{m};
	sessions.emplace_back(make_shared<HttpSession>(url));
	sessions.back()->getData([=](HttpSession &session, const vector<uint8_t> &v) {
		cb(v);
	});

}


}
