	attribute vec4 vertex;
	uniform mat4 matrix;

	void main() {
		gl_Position = matrix * vertex;
	}
