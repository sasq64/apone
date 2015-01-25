#ifdef GL_ES
	precision mediump float;
#endif
	uniform vec4 color;
	uniform sampler2D sTexture;
	varying vec2 UV;

	void main() {
		vec4 c = texture2D(sTexture, UV) * color;
		// NOTE: Should ONLY be enabled for draw calls requiring it!
		//if(c.a < 0.1)
		//	discard;
		gl_FragColor = c;
	}
