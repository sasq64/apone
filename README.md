GRAPPIX
=======

* Simple graphics library built on top of OpenGL 3.0 (and restricted to ES 2.0 profile)
* Allows for very simple programs
* Inspired by 8 bit basic computers

Used without other constructs, the line function
draws directly to buffer 0 (swap, draw, swap back)
line()
gfx.line()

When the screen has been flipped manually, double buffering
is turned on and flip() needs to be called in a loop

gfx.flip()

You can also change this manually

gfx.flags |= GFX:DOUBLE_BUFFER;


main() {

	for()
		line()
	return
}

	rendertarget rt(128, 128);
	rt.circle(64, 64, 60, 0x000080);
	rt.circle(92, 92, 32, 0x3030c0);
	screen.draw(rt);


BITMAP OPERATIONS
-----------------

* Normal operators add, sub with contstants or other bitmaps
* Split into many
* Concatenate into one, with or without padding to largest
* Make atlas




text("blabla")
