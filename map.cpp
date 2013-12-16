#include <grappix/grappix.h>
#include <coreutils/vec.h>
#include <vector>


using namespace std;
using namespace utils;
using namespace grappix;

#ifdef ANDROID
#define DATA_DIR "/sdcard/"
#else
#define DATA_DIR "data/"
#endif


//using namespace tween;
namespace grappix {
uint8_t *make_distance_map(uint8_t *img, int width, int height);
};
int main() {
	
	screen.open(640, 450, false);
	LOGD("Screen open");


	auto map = load_png(DATA_DIR "map.png");

	uint8_t data8[map.width() * map.height()];
	for(int i=0; i<map.size(); i++) {
		data8[i] = map[i] & 0xff000000 ? 0xff : 0x00;
	}

	uint8_t *ddata = make_distance_map(data8, map.width(), map.height());

	Texture mapTexture(ddata);

	GLuint id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, map.width(), map.height(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, ddata);

	//auto texture = texture(map);
	auto program = get_program(FONT_PROGRAM_DF);

//	glUseProgram(program);
	float scale = 1.0;
	float y = 0;
	while(screen.is_open()) {
		screen.clear();
		//screen.scale(scale*=1.01);
		screen.draw(mapTexture, 0, y);
		//y -= (scale / 2.0);
		//if(screen.key_pressed(Window::RIGHT)) {
		//	scrollTarget.x += 16;
		//	tween::to(0.5, { { layer.scrollx, scrollTarget.x }, { layer.scrolly, scrollTarget.y } } );
		//}
		screen.flip();
	}
	return 0;
}
