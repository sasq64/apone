GRAPPIX
=======

* Simple graphics library built on top of OpenGL 3.0 (and restricted to ES 2.0 profile)
* Allows for very simple programs
* Inspired by 8 bit basic computers


CLASS DESIGN/CODING STANDARD
============================

All classes should be designed for pass-by-value. This means that client code should
normally not need to use new or delete, and can program more like a scripting
language then traditional c++, including more functional programming. For example

``
function<Bitmap()> createFractalGenerator(int w, int h) {
	Bitmap bitmap(w, h);
	return [=]() -> Bitmap {
		for(int y=0; y<h; y++)
			for(int x=0; x<w; x++)
				bitmap[x+w*h] = someFunction(x,y);
		return bitmap;
	};
}
``


THE MAKEFILE MODULE SYSTEM
==========================

1. Include `config.mk`. This file tries figures out your environment and sets up
   default flags, compiler etc. It also makes sure you have the right SDKs etc and
   complains otherwise.

2. Include the modules you need.

3. Set your build variables;

4. Include `build.mk`
