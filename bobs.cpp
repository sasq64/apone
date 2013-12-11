#include <grappix.h>
using namespace utils;

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

class App {
public:
	App() : size {64, 64}, radius {size[0]/2}, sprite {size}, xy {0, 0}, xpos { screen.width() } {
		LOGD("Creating sprite");
		sprite.clear();
		sprite.circle(size/2, radius, 0x000020); // Outline
		sprite.circle(size/2, radius*0.9, 0x0000C0); // Main ball
		sprite.circle(size/2 + vec2f{radius*0.15f, -radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight
	};

	void update() {

		auto sz = vec2f(screen.size());
		auto scale = sz / 2.3;

		screen.clear();
		//screen.circle(80,80,10,0xff00ffff);
		vec2f xy2 = xy += {0.01, 0.03};
		for(int i=0; i<100; i++)
			screen.draw(sprite, (sin(xy2 += {0.156, 0.187}) + 1.0f) * scale);		
		screen.text(xpos-=2, 0.0, "Balls ON THE SCREEN!! A WEIRD WONDERFUL MACHINE!", 0xffffffff, 2.0);
		screen.flip();
	}

	vec2f size;
	float radius;
	Texture sprite;
	vec2f xy;
	//vec2f scale;
	int xpos;
};

void runMainLoop() {
	static App app;
	app.update();
}

int main() {
	
	LOGD("Main started");
	screen.open(640, 480, false);
	LOGD("Screen open");
	screen.renderLoop(runMainLoop);
	LOGD("Progam done");
	return 0;
}
