#include <grappix.h>

int cxx = 0;

int main() {
	screen.open(false);

	Texture t(load_png("data/font.png"));
	Texture t2 = t.clone();

	while(screen.is_open()) {
		//static int cxx = 0;
		screen.circle(cxx += 4, 100, 100, 0xff0000ff);
		screen.draw(t, 10, 10);
		screen.draw(t2, 30, 410);
		screen.flip();
	}
	return 0;
}
