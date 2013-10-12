GRAPPIX
=======

* Simple graphics library built on top of OpenGL 3.0 (and restricted to ES 2.0 profile)
* Allows for very simple programs
* Inspired by 8 bit basic computers


line(x...)
 EQU
x = make_line(x..)
draw(x)

Where x is a VBO with data and program

Need to define x

All data required to render or build a VBO
vertices, indexes, program, texture

STEP 1:
Define data so draw() is fast and simple

STEP 2:
Generate object, but rendering effeciency is key.

Variables of a VBO

Known uniforms: Allows you to scale, postion, colorize or rotate an unknown GL object

Program: Reprogram a known object
Attributes: Known object

Texture: Unknown object.

TWO DISTINCT PARTS; KNOWN/UNKNOWN

renderbuffer scroller(w,h);
scroller.set_program(p)

scroller.text("HELLO!")

screen.draw(scroller)

