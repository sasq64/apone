#ifndef DS_FIFO_H
#define DS_FIFO_H

#include <stdint.h>
#include <memory>

class Fifo {

public:
	Fifo(int size) : fifo(std::make_shared<_Fifo>(size)) {}
	//~Fifo();
	void putBytes(uint8_t *src, int bytelen) { fifo->putBytes(src, bytelen); }
	void putShorts(uint16_t *src, int shortlen) {
		putBytes((uint8_t*)src, shortlen*2);
	}
	int getBytes(uint8_t *dest, int bytelen) { return fifo->getBytes(dest, bytelen); }
	int getShorts(uint16_t *dest, int shortlen) {
		return getBytes((uint8_t*)dest, shortlen*2) / 2;
	}

	int filled() { return fifo->bufPtr - fifo->buffer; }
	int size() { return fifo->_size; }

	uint16_t* shortPtr() { return (uint16_t*)fifo->bufPtr; }
	uint8_t* bytePtr() { return (uint8_t*)fifo->bufPtr; }

	template <typename T> T *ptr() { return (T*)fifo->bufPtr;}

	uint16_t* shortBuffer() { return (uint16_t*)fifo->buffer; }
	uint8_t* byteBuffer() { return (uint8_t*)fifo->buffer; }
private:
	struct _Fifo {
		_Fifo(int size);
		~_Fifo();
		void putBytes(uint8_t *src, int bytelen);
		int getBytes(uint8_t *dest, int bytelen);
		int _size;
		int position;
		uint8_t *buffer;
		uint8_t *bufPtr;
	};
	std::shared_ptr<_Fifo> fifo;
};

#endif
