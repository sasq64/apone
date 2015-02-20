# GRAPPIX

* Simple graphics library built with OpenGL 3.0 (and restricted to ES 2.0 profile)
* Allows for very short programs
* Supports *Linux X11*, *Raspberry PI Console*, *Mac OS*, *Android* and *Emscripten*.
* Uses C++11 

## PREREQUISITES

* gcc-4.7+ or clang-3.2+
* freetype
* glfw3 (PC)
* glew (PC)

### Ubuntu
```
sudo apt-get install libglfw-dev libglew-dev
```

### Mac OS (brew)
```
brew install glew glfw3 
```

## SIMPLE EXAMPLE
```C++
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
```

## THE RENDER LOOP

In *grappix*, you specify the render loop through a callback function. This function will be called once "per frame", meaning when the screen needs to be re-rendered. The reason for using a callback and not let the application implement it's own render loop is mainly one; *Emscripten*. Javascript is event based and single threaded, so we can't run our own continuous loop.

The biggest implication of this is; any variables defined in *main()* will be *out of scope* when the render loop is actually executing. This means you must either rely on those variables being *copied* into your lambda, like in the above example -- or you must define them as static, so they wont be destroyed when main() ends.

## READING INPUT

```C++
auto k = screen.get_key();
if(k == Window::LEFT)
    moveLeft();
```

### PLATFORM SUPPORT

#### Raspberry PI

grappix does not use X on Raspberry PI, you can run your applications directly from the console. Your application will always be full screen, and while running will grab exclusive access to the keyboard.

#### Emscripten

...