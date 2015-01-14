#include <grappix/grappix.h>

using namespace grappix;

int main() {
	screen.open(false);
	screen.render_loop([=](uint32_t delta) mutable {
		screen.clear();
		screen.circle(100, 100, 50, 0xff0000ff);
		screen.flip();
	});
	return 0;
}
