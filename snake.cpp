#include <grappix.h>
#include <vector>
#include <cstdlib>

using namespace std;
using namespace utils;

int main() {

	screen.open(false);

	static const vec2i pfSize {32, 18};
	static const vec2i pfpos { 50, 50 };
	static const vec2i border { 100, 100 };

	vec2i screenSize = vec2i(screen.size());
	vec2i tileSize = (screenSize - border) / pfSize;


	const int w = pfSize.x;
	const int h = pfSize.y;

	vector<int> playField(w*h);

	playField[rand() % (w*h)] = -1;

	vec2f pos = {5,5};
	int score = 0;

	int d = 2;
	static const vector<vec2f> adds { {0,-1}, {1,0}, {0,1}, {-1,0} };
	bool noKey = true;
	int speed = 50;
	int delay = speed;
	int snakeLen = 100;

	bool gameOver = false;
	int scrollx = screen.width();

	srand(clock());
	bool doStep = false;
	while(screen.is_open()) {

		if(!gameOver) {
			if(screen.key_pressed(window::LEFT)) {
				if(noKey) {
					d = (d+3) % 4;
					doStep = true;
				}
				noKey = false;
			} else if(screen.key_pressed(window::RIGHT)) {
				if(noKey) {
					d = (d+1) % 4;
					doStep = true;
				}
				noKey = false;
			} else
				noKey = true;

			if((delay-=4) <= 0 || doStep) {
				doStep = false;
				pos += adds[d];
				if(pos.x < 0 || pos.y < 0 || pos.x >= w || pos.y >= h || playField[pos.x + pos.y * w] > 0) {
					gameOver = true;
				}

				snakeLen += 4;

				if(playField[pos.x + pos.y * w] == -1) {
					if(speed > 0)
						speed--;
					score++;
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
		}
		screen.clear();

		screen.scale(1.0);
		screen.rectangle(pfpos - 10, tileSize.x * w + 20, tileSize.y * h + 20, 0x00a000);
		screen.rectangle(pfpos, tileSize.x * w, tileSize.y * h, 0x000000);
		int i = 0;
		for(auto &p : playField) {
			if(p) {
				vec2i pos { i%w, i/w };
				if(p == -1) {
					int r = tileSize.x/3;
					screen.circle(pos * tileSize + pfpos + r, r, 0x00ff00);
				} else {
					screen.scale(0.9 - 0.7 * p/snakeLen);
					screen.rectangle(pos * tileSize + pfpos, tileSize, blend(0x004000, 0x00ff00, (float)p/snakeLen));
					p++;
					if(p == snakeLen)
						p = 0;
				}
			}
			i++;
		}

		screen.text(10, 5, format("Score:%d", score), 0xff00ff00, 1.0);

		if(gameOver) {
			screen.text(scrollx-=5, screen.height()/2 - 50, "Game Over", 0x80c0ffc0, 10.0);
		}

		screen.flip();
	}

	return 0;
}