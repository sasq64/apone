#include <grappix.h>

int cxx = 0;

int main() {
	screen.open(false);

	while(screen.is_open()) {
		screen.clear();
		screen.circle(cxx += 4, 100, 100, 0xff0000ff);
		screen.text("Circle",screen.width()-120,screen.height()-50,0xffffffff, 2.0);
		screen.flip();
	}
	return 0;
}
