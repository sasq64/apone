#include <grappix.h>

using namespace utils;

static bitmap data;

int main() {
	
	screen.open(true);

	LOGD("Screen is open");

	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	renderbuffer sprite(size);
	LOGD("Clearing sprite");
	sprite.clear();
	sprite.circle(size/2, radius, 0x000020); // Outline
	sprite.circle(size/2, radius*0.9, 0x0000C0); // Main ball
	sprite.circle(size/2 + vec2f{radius*0.15f, radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight

	// Loop and render ball worm
	vec2f xy{0, 0};
	auto scale = vec2f(screen.size()) / 2.3;
	int xpos = screen.width();

	//data = read_png_file("data/rpgtiles.png");

	LOGD("Starting mainloop (%dx%d)", screen.width(), screen.height());

	while(screen.is_open()) {
		screen.clear();
		vec2f xy2 = xy += {0.01, 0.03};
		for(int i=0; i<100; i++)
			screen.draw((sin(xy2 += {0.156, 0.187}) + 1.0f) * scale, sprite);		
		screen.text((xpos-=8), 0, "BALLS ON THE SCREEN!!", 0xc080c0ff, 6.0);
		LOGD("xy  %f %f", xy.x, xy.y);
		screen.flip();
	}
	return 0;
}
