#include <grappix.h>
using namespace utils;

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

static std::function<void(void)> mainLoop;

void runMainLoop() {
	mainLoop();
}

int main() {
	
	LOGD("Main started");
	screen.open(true);
	LOGD("Screen open");

	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	texture sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000020); // Outline
	sprite.circle(size/2, radius*0.9, 0x0000C0); // Main ball
	sprite.circle(size/2 + vec2f{radius*0.15f, -radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight

	vec2f xy{0, 0};
	auto scale = vec2f(screen.size()) / 2.3;
	int xpos = screen.width();

	mainLoop = [&]() {
		LOGD("Main loop");
		screen.clear();
		screen.circle(80,80,10,0xff00ffff);
		vec2f xy2 = xy += {0.01, 0.03};
		for(int i=0; i<100; i++)
			screen.draw(sprite, (sin(xy2 += {0.156, 0.187}) + 1.0f) * scale);		
		//screen.text(xpos-=8, (sin(xy2.y)+1)*320, "BALLS ON THE SCREEN!!", 0x8080c0ff, 10.0);
		screen.flip();
	};

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(runMainLoop, 30, false);
#else
	// Loop and render ball worm
	while(screen.is_open()) {
		mainLoop();
	}
#endif
	LOGD("Progam done");
	return 0;
}
