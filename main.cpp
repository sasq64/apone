#include <grappix.h>

using namespace utils;

int main() {
	
	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	renderbuffer sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000000); // Outline
	sprite.circle(size/2, radius-10, 0xC00000); // Main ball
	sprite.circle(size/2 + vec2f{20,20}, radius-50, 0xFF4040); // Hilight

	// Loop and render ball worm
	vec2f xy{0, 0};
	auto scale = vec2f(screen.size()) / 2.3;
	while(screen.is_open()) {
		screen.clear();
		vec2f xy2 = xy;
		for(int i=0; i<100; i++)
			screen.draw((sin(xy2 += {0.09, 0.13}) + 1.0f) * scale, sprite);
		xy += {0.01, 0.03};
		screen.flip();
	}
	return 0;
}
