#include <grappix/grappix.h>
#include <vector>

using namespace std;
using namespace utils;
using namespace grappix;

struct App {

		const int w = 32;
		const int h = 18; // Size of playfield
		const vec2i pfpos { 50, 50 }; // Position on screen
		vec2i tileSize = (vec2i(screen.size()) - vec2i{100,100}) / vec2i{w,h};
		vector<int> playField;

		int score;
		int hiscore;
		vec2f pos; // position of snake
		int dir; // direction of snake
		int speed; // speed of snake
		int delay = speed;
		int snakeLen = 100; // length of snake
		bool gameOver = false;
		int scrollx = screen.width(); // Game over scroller position


	App() : playField (w*h) {	
		hiscore = 0;
		start();
	}

	void start() {
		score = 0;
		pos = {5,5};
		dir = 2; 
		speed = 50;
		delay = speed;
		snakeLen = 100;
		gameOver = false;
		scrollx = screen.width();
		srand(clock());
		memset(&playField[0], 0, sizeof(int) * w * h);
		// Create initial apple
		playField[rand() % (w*h)] = -1;
	}

	void update() {
		// Logic
		if(!gameOver) {
			if((delay-=4) <= 0) {
				// Change direction depending on key
				static const vector<vec2f> adds { {0,-1}, {1,0}, {0,1}, {-1,0} };
				auto k = screen.get_key();
				if(k == Window::LEFT) dir = (dir+3) % 4;
				else if(k == Window::RIGHT) dir = (dir+1) % 4;
				auto m = screen.get_click();
				if(m != Window::NO_CLICK) {
					if(m.x < screen.width()/2) dir = (dir+3) % 4;
					else dir = (dir+1) % 4;
				}
				// Move snake forward
				pos += adds[dir];
				// Check collision with walls or self
				gameOver = (pos.x < 0 || pos.y < 0 || pos.x >= w || pos.y >= h || playField[pos.x + pos.y * w] > 0);
				// Grow snake
				snakeLen += 4;
				// Check apple
				if(playField[pos.x + pos.y * w] == -1) {
					if(speed > 0)
						speed--;
					score++;
					// Add new apple
					while(true) {
						auto applePos = rand() % (w*h);
						if(playField[applePos] == 0) {
							playField[applePos] = -1;
							break;
						}
					}
				}
				playField[pos.x + pos.y * w] = 1;
				delay += speed;
			}
		} else {
			auto k = screen.get_key();
			if(k == Window::SPACE || k == Window::ENTER) {
				start();
			}
		}

		// Rendering
		screen.clear();
		screen.rectangle(pfpos - 10, tileSize.x * w + 20, tileSize.y * h + 20, 0x00a000);
		screen.rectangle(pfpos, tileSize.x * w, tileSize.y * h, 0x000000);
		int i = 0;
		for(auto &p : playField) {
			if(p) {
				vec2i pos { i%w, i/w };
				if(p == -1) {
					screen.circle(pos * tileSize + pfpos + tileSize.x/3, tileSize.x/3, 0x90ffa0);
				} else {
					screen.rectangle(pos * tileSize + pfpos, tileSize, blend(0x006000, 0x60ff60, (float)p/snakeLen), 0.9 - 0.7 * p/snakeLen);
					if(p++ == snakeLen)
						p = 0;
				}
			}
			i++;
		} 
		screen.text(format("Score:%d", score), 40, 0, 0xff00ff00, 1.4);
		screen.text(format("Hicore:%d", hiscore), screen.width()-200, 5, 0xff00ff00, 1.0);
		if(gameOver) {
			screen.text("Game Over", scrollx-=5, 0, 0x80c0ffc0, 20.0);
			if(score > hiscore)
				hiscore = score;
		}
		screen.flip();
	}
};

void runMainLoop(uint32_t delta) {
	static App app;
	app.update();
}

int main() {
	screen.open(800, 450, false);
	LOGD("Screen open");
	screen.render_loop(runMainLoop);
	return 0;
}