
#include <fusefs/fusefs.h>

#include <grappix/grappix.h>
#include <coreutils/file.h>
#include <vector>

using namespace grappix;
using namespace utils;

void init_fuse_grappix() {

	auto &fuse = FuseFS::getInstance();

	fuse.addFile("screen.png", [=]() -> vector<uint8_t> {
		LOGD("Generating screen in %d", std::this_thread::get_id());
		image::bitmap pix;
		screen.run_safely([&]() {
			pix = screen.get_pixels();
		});

		image::save_png(pix, "dummy.png");
		File f { "dummy.png" };
		vector<uint8_t> v8(f.getSize());
		f.read(&v8[0], v8.size());
		f.close();
		f.remove();
		return v8;
	});

	fuse.addFile("shader.glsl",
		[=]() -> vector<uint8_t> {
			vector<uint8_t> v;
			screen.run_safely([&]() {
				auto &p = get_program(TEXTURED_PROGRAM);
				auto &s = p.fSource;
				v = vector<uint8_t>(s.begin(), s.end());
				LOGD("%d vs %d\n%s\n", v.size(), s.size(), s);
			});
			return v;
		},

		[=](const vector<uint8_t> &v) {
			auto s = std::string(v.begin(), v.end());
			screen.run_safely([&]() {
				auto &p = get_program(TEXTURED_PROGRAM);
				LOGD("New shader:\n%s\n", s);
				auto saved = p.fSource;
				try {
					p.setFragmentSource(s);
				} catch(shader_exception &e) {
					p.setFragmentSource(saved);
				}
			});
		}
	);

	fuse.mount("/home/sasq/fuse");


}