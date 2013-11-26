#include <grappix.h>
#include <vector>

using namespace std;
using namespace utils;

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
				if(k == window::LEFT) dir = (dir+3) % 4;
				else if(k == window::RIGHT) dir = (dir+1) % 4;
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
			if(k == window::SPACE || k == window::ENTER) {
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
		screen.text(10, 5, format("Score:%d", score), 0xff00ff00, 1.0);
		screen.text(screen.width()-200, 5, format("Hicore:%d", hiscore), 0xff00ff00, 1.0);
		if(gameOver) {
			screen.text(scrollx-=5, screen.height()/2 - 50, "Game Over", 0x80c0ffc0, 10.0);
			if(score > hiscore)
				hiscore = score;
		}
		screen.flip();
	}
};

void runMainLoop() {
	static App app;
	app.update();
}

int main() {
	screen.open(800, 450, false);
	LOGD("Screen open");
	screen.renderLoop(runMainLoop);
	return 0;
}