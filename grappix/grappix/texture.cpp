#include "GL_Header.h"

#include "texture.h"

#include <coreutils/log.h>

using namespace std;
using namespace image;

namespace grappix {

Texture::Texture(const bitmap &bm) {

	_width = bm.width();
	_height = bm.height();

	tref = make_shared<texref>();

	glBindTexture(GL_TEXTURE_2D, tref->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.flipped());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

}

Texture::Texture(uint8_t *data, unsigned int w, unsigned int h) {
	_width = w;
	_height = h;

	LOGD("%dx%d", _width, _height);

	tref = make_shared<texref>();

	glBindTexture(GL_TEXTURE_2D, tref->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
}

Texture::Texture(unsigned int width, unsigned int height, Format fmt) {

	_width = width;
	_height = height;

	// Create and bind a new framebuffer with emtpy attachment points (not yet useable)
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	tref = make_shared<texref>();
	glBindTexture(GL_TEXTURE_2D, tref->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	LOGD("texid %d", tref->id);

	// Bind the Texture to the COLOR_ATTACHMENT of our framebuffer. This would not work on the default (0) framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tref->id, 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbuf);

	//LOGD("ERROR %d", glGetError());
	//GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//LOGD("STATUS %d", status);
/*if(status != GL_FRAMEBUFFER_COMPLETE) {
	glfwSleep(100);
}
*/
}

}