#ifdef GL_ES
	precision mediump float;
#endif
	//uniform vec4 fColor;
	uniform sampler2D sTexture;
	varying vec2 UV;

	void main() {	
		gl_FragColor = texture2D(sTexture, UV);
	}
