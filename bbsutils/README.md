Console
=======

* Is a grid of *Tiles*
* Each Tile has a character, a foreground color and a background color.
* The console keeps track of the clients screen contents
* The console keeps track of the clients real cursor position
* A *refresh* means to send the necessary (and preferably minimal) set of characters and control codes to mirror the internal representation on to the client screen.
* Refreshing should restore the real cursor position.
* 

Example

console.write("One line\nNext line");

for(int y=0; y<10; y++)
	console.put(0,y, format("Line%d", y));
console.flush();
