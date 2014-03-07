
#include <grappix/grappix.h>
using namespace grappix;

int main(int argc, char **argv) {
	screen.open(640, 480, false);
	double x = 0;
	screen.render_loop([=](uint32_t delta) mutable {
		screen.clear();
		screen.circle(x += (delta/10.0), 200, 50, 0xff00ff);
		screen.text("CIRCLE", 10, 0, 0xffffffff, 2.0);
		screen.flip();
	});
	return 0;
}
