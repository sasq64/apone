
#include <grappix.h>
#include <vector>
#include <cstdlib>

using namespace std;
using namespace utils;

int main() {

	vec2f screenSize = vec2f(screen.size());
	vec2i pfSize {32, 18};
	vec2f border { 100, 100 };
	vec2f tileSize = (screenSize - border) / pfSize;
	tileSize.x = (int)tileSize.x;
	tileSize.y = (int)tileSize.y;

	srand(0);

	int w = pfSize.x;
	int h = pfSize.y;

	vector<int> playField(w*h);

	playField[rand() % (w*h)] = -1;

	vec2f pos = {5,5};
	int score = 0;

	int d = 2;
	vector<vec2f> adds { {0,-1}, {1,0}, {0,1}, {-1,0} };
	bool noKey = true;
	int speed = 10;
	int delay = speed;
	int snakeLen = 100;

	bool gameOver = false;

	while(screen.is_open()) {

		if(!gameOver) {
			if(screen.key_pressed(window::LEFT)) {
				if(noKey)
					d--;
				noKey = false;
			} else if(screen.key_pressed(window::RIGHT)) {
				if(noKey)
					d++;
				noKey = false;
			} else
				noKey = true;

			d = (d+4) % 4;

			if(delay-- == 0) {
				pos += adds[d];
				if(pos.x < 0 || pos.y < 0 || pos.x >= w || pos.y >= h || playField[pos.x + pos.y * w] > 0) {
					gameOver = true;
				}

				snakeLen += 4;

				if(playField[pos.x + pos.y * w] == -1) {
					snakeLen += 24;
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
				delay = speed;
			}
		}
		screen.clear();

		vec2f pfpos { 50, 50 };
		screen.scale(1.0);
		screen.rectangle(pfpos - 10, tileSize.x * w + 20, tileSize.y * h + 20, 0x00a000);
		screen.rectangle(pfpos, tileSize.x * w, tileSize.y * h, 0x000000);
		int i = 0;
		for(auto &p : playField) {
			if(p) {
				vec2f pos { (float)(i%w), (float)(i/w) };
				if(p == -1) {
					float r = tileSize.x/3;
					screen.circle(pos * tileSize + pfpos + r, r, 0x00ff00);
				} else {
					if(gameOver) {
						screen.scale(1.2 - (float)p/snakeLen);
						screen.rectangle(pos * tileSize + pfpos, tileSize, blend(0x004000, 0x00ff00, (float)p/snakeLen));
					} else  {
						screen.scale(1.2 - (float)p/snakeLen);
						screen.rectangle(pos * tileSize + pfpos, tileSize, blend(0x004000, 0x00ff00, (float)p/snakeLen));
						p++;
						if(p == snakeLen)
							p = 0;
					}
				}
			}
			i++;
		}

		screen.text(10, 5, format("Score:%d", score), 1.0);
		screen.flip();
	}

	return 0;
}