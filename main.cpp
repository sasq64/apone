#include <grappix.h>

int main() {
	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	renderbuffer sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000000); // Outline
	sprite.circle(size/2, radius-10, 0xc00000); // Main ball
	sprite.circle(size/2 + vec2f{20,-20}, radius-50, 0xff4040); // Hilight

	// Loop and render ball worm
	vec2f xy{0, 0};
	while(screen.is_open()) {
		screen.clear();
		for(int i=0; i<100; i++)
			screen.draw((sin(xy + vec2f{i*0.09f, i*0.13f}) + 1.0f) * screen.size() / 2.3, sprite);
		xy += {0.01, 0.033};
		screen.flip();
	}
	return 0;
}
