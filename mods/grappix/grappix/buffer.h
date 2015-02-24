#ifndef BUFFER_H
#define BUFFER_H

#include <memory>
#include <cstdint>

class Buffer {

	using GLuint = uint32_t;


	Buffer();
	Buffer(GLuint bufid);
	void bind();

private:
	struct bufref {
		bufref();
		bufref(GLuint id) : id(id) {}
		~bufref();
		GLuint id;
	};

	std::shared_ptr<bufref> buf;
	std::shared_ptr<bufref> elemBuf;

};

#endif // BUFFER_H