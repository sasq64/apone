#include <grappix/grappix.h>

using namespace grappix;


int main() {
	screen.open(false);
	Font font("data/ObelixPro.ttf", 18);
	int cxx = 0;
	auto bm = load_png("data/tiles.png");
	Texture t(bm);

	screen.render_loop([=](uint32_t) mutable {
		screen.clear();
		screen.draw(t);
		screen.circle(cxx += 4, 200, 180, 0xff0000ff);
		screen.text(font, "Circle",screen.width()-120,screen.height()-50,0xffffffff, 2.0);
		screen.line(10,10,500,40,0xff00ff00);
		screen.flip();
	});
	return 0;
}
