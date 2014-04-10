	attribute vec4 vertex;
	attribute vec2 uv;

	uniform vec4 vPosition;
	uniform vec4 vScale;
	uniform vec4 vScreenScale;
	uniform float vUniformZ;

	varying vec2 UV;

	//uniform vec2 uvs[4];

	void main() {
		vec4 v = vertex * vScale + vPosition;
		gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, vUniformZ, 1);
		UV = uv;
	}
