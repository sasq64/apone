#ifdef GL_ES
	precision mediump float;
#endif
	attribute vec4 vertex;
	attribute vec2 uv;

	uniform mat4 matrix;

	//uniform vec4 vPosition;
	//uniform vec4 vScale;
	//uniform vec4 vScreenScale;

	varying vec2 UV;

	void main() {
		//vec4 v = vertex * vScale + vPosition;
		//gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, 0, 1);
		gl_Position = matrix * vertex;
		UV = uv;
	}
