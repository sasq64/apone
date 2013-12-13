	attribute vec2 vertex;
	uniform vec2 vScreenScale;
	uniform vec2 vScale;
	uniform vec2 vPosition;
	//uniform float rotation;
	void main() {
		vec2 v = vertex * vScale + vPosition;
		gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, 0, 1);
	}
