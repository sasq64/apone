#include <grappix.h>

using namespace utils;

static bitmap data;

int main() {
	
	screen.open(true);

	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	renderbuffer sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000020); // Outline
	sprite.circle(size/2, radius*0.9, 0x0000C0); // Main ball
	sprite.circle(size/2 + vec2f{radius*0.15f, radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight

	vec2f xy{0, 0};
	auto scale = vec2f(screen.size()) / 2.3;
	int xpos = screen.width();

	// Loop and render ball worm
	while(screen.is_open()) {
		screen.clear();
		vec2f xy2 = xy += {0.01, 0.03};
		for(int i=0; i<100; i++)
			screen.draw((sin(xy2 += {0.156, 0.187}) + 1.0f) * scale, sprite);		
		screen.text(xpos-=8, 150, "BALLS ON THE SCREEN!!", 0x80ffc0ff, 10.0);
		screen.flip();
	}
	return 0;
}
