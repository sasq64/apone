#ifndef GRAPPIX_RENDERBUFFER_H
#define GRAPPIX_RENDERBUFFER_H

#include "basic_buffer.h"

class renderbuffer : public basic_buffer {
public:
	template <typename T> renderbuffer(T size) : renderbuffer(size[0], size[1]) {}
	renderbuffer(int width, int height);
	unsigned int texture() { return texture_id; }

private:
	unsigned int texture_id;
};

#endif // GRAPPIX_RENDERBUFFER_H
