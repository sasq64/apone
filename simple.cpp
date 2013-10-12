#include <grappix.h>
int main() {
	screen.open();
	int x = 0;
	while(screen.is_open()) {
		screen.circle(x += 4, 250, 100, 0xffff00ff);
		screen.flip();
	}
	return 0;
}
