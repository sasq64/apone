
#include "buffer.h"
#include "GL_Header.h"

Buffer::Buffer() {
}

Buffer::bufref::bufref() {
	//LOGD("GEN");
	glGenBuffers(1, &id);
}

Buffer::bufref::~bufref() {
	//LOGD("DEL");
	glDeleteBuffers(1, &id);
}
