#pragma once
#include "web.h"
#include <coreutils/file.h>
#include <coreutils/fifo.h>
#include <functional>

namespace webutils {

template <> struct Sink<utils::File>
{
	Sink(const utils::File &f) : file(f) {}

	size_t write(uint8_t *ptr, size_t size) { 
		file.write(ptr, size);
		return size;
   	}

	utils::File& result() { return file; }
	utils::File file;
};

template <> struct Sink<utils::Fifo<uint8_t>>
{
	Sink(int size = 1024*128) : fifo(size) {}

	size_t write(uint8_t *ptr, size_t size) {
		if(fifo.left() < (int)size) {
			LOGD("Fifo full, pausing");
			paused = true;
		   	return 0;
		}
		LOGD("Put %d bytes", size);
		fifo.put(ptr, size);
		return size;
	}

	utils::Fifo<uint8_t>& result() { return fifo; }
	bool paused = false;
	utils::Fifo<uint8_t> fifo;
};

using Callback = std::function<bool(uint8_t*, size_t)>;

template <> struct Sink<Callback>
{
	Sink(const Callback &cb) : cb(cb) {}

	size_t write(uint8_t *ptr, size_t size) {
		if(cb)
			cb(ptr, size);
		return size;
	}

	Callback &result() {
		return cb;
	}

	Callback cb;
};

} // namsepace
