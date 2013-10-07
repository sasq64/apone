#include <grappix.h>

using namespace utils;

int main() {
	
	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	renderbuffer sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000020); // Outline
	sprite.circle(size/2, radius*0.90, 0x0000C0); // Main ball
	sprite.circle(size/2 + vec2f{radius*0.15f, radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight

	// Loop and render ball worm
	vec2f xy{0, 0};
	auto scale = vec2f(screen.size()) / 2.3;
	int xpos = 1600;
	while(screen.is_open()) {
		screen.clear();
		vec2f xy2 = xy += {0.01, 0.03};
		for(int i=0; i<100; i++)
			screen.draw((sin(xy2 += {0.056, 0.291}) + 1.0f) * scale, sprite);		
		float zoom = 10;//(sin(xpos/235.0)+1.5)*10.0;
		screen.text((xpos-=4), 600, "BALLS ON THE SCREEN!!", 0x808040ff, zoom);
		screen.flip();
	}
	return 0;
}
