
#include "renderbuffer.h"
#include <GL/glew.h>


renderbuffer::renderbuffer(const bitmap &bm) {

	_width = bm.width();
	_height = bm.height();

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

}


renderbuffer::renderbuffer(int width, int height) {

	_width = width;
	_height = height;

	// Create and bind a new framebuffer with emtpy attachment points (not yet useable)
	//GLuint fbuf;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// Create an empty texture with no filtering
	//GLuint renderedTexture;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Bind the texture to the COLOR_ATTACHMENT of our framebuffer. This would not work on the default (0) framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbuf);

	/*
GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
if(status != GL_FRAMEBUFFER_COMPLETE) {
	glfwSleep(100);
}
*/

}
