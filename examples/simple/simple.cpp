#include <grappix/grappix.h>
#include <tween/tween.h>

using namespace grappix;
using namespace tween;

int main() {

	static Color color = Color::BLACK;
	static float size = 50.0;

	Tween::make().repeating().to(size, 200).seconds(1.0);
	Tween::make().to(color, Color::BLUE).seconds(3.0);

	screen.open(false);
	screen.render_loop([=](uint32_t delta) mutable {
		screen.clear();
		screen.circle(screen.width()/2, screen.height()/2, size, color);
		screen.flip();
	});
	return 0;
}
