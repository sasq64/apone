
#include "frame.h"
#include "GL_Header.h"

namespace grappix {

void Frame::set(const RenderTarget &target) {
	glEnable(GL_SCISSOR_TEST);
	glScissor(r.x, target.height()-r.h-r.y, r.w, r.h);
}

void Frame::unset() {
	glDisable(GL_SCISSOR_TEST);
}

}