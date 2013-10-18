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



Tiles & Playfields


TileArea ta(tileSet);

ta[x+y*w] = 0;
ta.render();

class Tween {
};

PlayField pf(tileSet);
	moveSprite(int no, function<vec2f(float t)>)

pf.putSprite(0, x, y);
pf.moveSpriteTo(0, x2, y2);
pf.moveSprite(0, generator);

pf.moveSprite(1, Tween({pos, 0}, {easeIn, true});

pf.render();

class TweenArg {
TweenARg(string, T)
}

Tween(obj, time, vector<TweenArg> args)


Tween(Sprite, t, vector<TweenArg<Sprite>> args)
