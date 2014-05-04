	attribute vec4 vertex;
	attribute vec2 uv;

	uniform vec4 vPosition;
	uniform vec4 vScale;
	uniform vec4 vScreenScale;
	uniform float vUniformZ;
	uniform mat4 matrix;

	varying vec2 UV;

	//uniform vec2 uvs[4];

	void main() {
		//vec4 v = vertex * vScale + vPosition;
		//gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, 0.5, 1);
		////vec4 v = vertex;// * vScale + vPosition;
		gl_Position = matrix * vertex;
		//v.z = 0.5;
		//v = v * vScreenScale;
		//gl_Position = v; //vec4(v.x, v.y, v.z, v.w);
		UV = uv;
	}
