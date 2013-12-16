#include <grappix/grappix.h>

using namespace grappix;

int cxx = 0;

int main() {
	screen.open(false);

	{
		Font font("data/ObelixPro.ttf", 20, 1);
		auto bm = load_png("data/tiles.png");
		Texture t(bm);

		screen.render_loop([=]() {
			LOGD("Loop");
			screen.clear();
			screen.draw(t);
			screen.circle(cxx += 4, 200, 180, 0xff0000ff);
			screen.text(font, "Circle",screen.width()-120,screen.height()-50,0xffffffff, 2.0);
			screen.flip();
		});
	}
	return 0;
}
