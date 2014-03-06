#include <grappix/grappix.h>

using namespace utils;
using namespace grappix;

int main() {

	screen.open(640, 480, false);
	LOGD("Screen open");

	float radius = 32;
	vec2f size {radius*2, radius*2};
	Texture sprite { size };

	screen.call_repeatedly([]() {
		LOGD("HELLO!");
	}, 1000);

	sprite.clear();
	sprite.circle(size/2, radius, 0x000020); // Outline
	sprite.circle(size/2, radius*0.9, 0x0000C0); // Main ball
	sprite.circle(size/2 + vec2f{radius*0.15f, -radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight
	vec2f xy {0, 0};
	float textx = screen.width();
	const int count = 100;
	float zoom = 1.0;
	screen.render_loop([=](uint32_t d) mutable {
		static std::vector<vec2f> v(count);
		auto scale = vec2f(screen.size()) / 2.3;
		screen.clear();
		vec2f xy2 = xy += {0.001f * d, 0.003f * d};
		for(int i=0; i<count; i++)
			v[i] = (sin(xy2 += {0.156, 0.187}) + 1.0f) * scale;
		screen.draw_texture(sprite.id(), &v[0][0], count, sprite.width(), sprite.height());
		screen.text("Balls on the screen!! A Weird Wonderful Machine!", textx-=(d/4.0), -zoom*8, 0xffffffff, zoom *= 1.005);
		screen.flip();
	});
	return 0;
}
