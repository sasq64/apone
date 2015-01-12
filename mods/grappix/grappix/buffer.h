#ifndef BUFFER_H
#define BUFFER_H

#include "GL_Header.h"
#include <memory>
class Buffer {
	Buffer();
	Buffer(GLuint bufid);
	void bind();

private:
	struct bufref {
		bufref() {
			//LOGD("GEN");
			glGenBuffers(1, &id);
		}
		bufref(GLuint id) : id(id) {}
		~bufref() {
			//LOGD("DEL");
			glDeleteBuffers(1, &id);
		}
		GLuint id;
	};

	std::shared_ptr<bufref> buf;
	std::shared_ptr<bufref> elemBuf;

};

#endif // BUFFER_H