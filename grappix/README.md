# GRAPPIX

* Simple graphics library built with OpenGL 3.0 (and restricted to ES 2.0 profile)
* Allows for very short programs
* Supports *Linux*, *Mac OS*, *Android* and *Emscripten*.
* Uses C++11 

## PREREQUISITES

* gcc-4.7+ or clang-3.2+
* libpng
* freetype
* glfw 2
* glew

### Ubuntu
```
sudo apt-get install libfreetype6-dev libpng12-dev libglfw-dev libglew-dev
```

### Mac OS (brew)
```
brew install freetype glew libpng homebrew/versions/glfw2 
```
## QUICKSTART

```Shell
$ git clone http://github.com/sasq64/cpp-mods.git
$ git clone http://github.com/sasq64/grappix.git
$ cp -a grappix/quickstart .
$ cd quickstart
$ ls
Makefile test.cpp
$ cat test.cpp

#include <grappix/grappix.h>
using namespace grappix;

int main(int argc, char **argv) {
	screen.open(640, 480, false);
	double x = 0;
	screen.render_loop([=](uint32_t delta) mutable {
		screen.clear();
		screen.circle(x += (delta/10.0), 200, 50, 0xff00ff);
		screen.text("CIRCLE", 10, 0, 0xffffffff, 2.0);
		screen.flip();
	});
	return 0;
}
$ make
$ ./test
```

## CLASS DESIGN/CODING STANDARD

All grappix classes are designed for pass-by-value. This means that client code should
normally not need to use new or delete, and can program more like a scripting
language then traditional c++, including more functional programming.
