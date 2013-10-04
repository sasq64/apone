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

	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.width(), bm.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &bm[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Loop and render ball worm
	vec2f xy{0, 0};
	auto scale = vec2f(screen.size()) / 2.3;
	while(screen.is_open()) {
		screen.clear();
		for(int i=0; i<350; i++)
			screen.draw((sin(xy + vec2f{i*0.09f, i*0.13f}) + 1.0f) * scale, sprite);
		xy += {0.01, 0.03};
		screen.draw_texture(texid, 10, 10, bm.width(), bm.height());
		screen.flip();
	}
	return 0;
}
