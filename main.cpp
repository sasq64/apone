#include <grappix.h>

using namespace utils;

bitmap read_png_file(const std::string &file_name);

int main() {
	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	renderbuffer sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000000); // Outline
	sprite.circle(size/2, radius-10, 0xC00000); // Main ball
	sprite.circle(size/2 + vec2f{20,-20}, radius-50, 0xFF4040); // Hilight

	auto bm = read_png_file("platlogo.png");
	for(int i=0; i<bm.size(); i++) {
		if(bm[i] & 0xff000000)	
			bm[i] = (bm[i] & 0x00ffffff) | 0x80000000;
	}
	renderbuffer logo(bm);

	// Loop and render ball worm
	vec2f xy{0, 0};
	auto scale = vec2f(screen.size()) / 2.3;
	while(screen.is_open()) {
		screen.clear();
		for(int i=0; i<350; i++)
			screen.draw((sin(xy + vec2f{i*0.09f, i*0.13f}) + 1.0f) * scale, sprite);
		xy += {0.01, 0.03};
		screen.draw(10, 10, logo);
		screen.flip();
	}
	return 0;
}
