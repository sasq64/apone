# APONE

## Introduction

Apone is a C++ *library*, or *collection of code modules* to help you write modern, portable applications -- specifially ones using graphics and audio.

```c++
#include <grappix/grappix.h>

using namespace grappix;

int main() {
    screen.open(false);
    screen.render_loop([=](uint32_t delta) mutable {
        screen.clear();
        screen.circle(100, 100, 0xff0000ff);
        screen.flip();
    });
    return 0;
}
```

## Platform support

### Posix Desktop (Linux, Mac)

### Windows

### Raspberry PI

#### Cross compile

Requires

* Raspberry PI cross compiler in your path (arm-linux-gnueabihf-gcc etc)
* A copy of lib, usr/lib, usr/include and vc from a Rasbian Jessie installation

#### Native compile

* Similar requirements as Desktop/Linux, but you need at least Rasbian Jessie (not Wheezy) for
  a modern enough compiler.
* Also note that compiling on the Raspberry is very slow.

### Android

Requires

* Android NDK r9+
* Android SDK api 19+
* ant

### Emscripten

Requires: *emscripten*

# Quickstart

```
```
