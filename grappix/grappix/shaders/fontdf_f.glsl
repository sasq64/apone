#ifdef GL_ES
	precision mediump float;
#endif
	uniform vec4 color;
	uniform float vScale;
	uniform sampler2D sTexture;
	//uniform float smoothing;
	varying vec2 UV;

	vec3 glyph_color    = vec3(0.0,1.0,0.0);
	const float glyph_center   = 0.50;
	vec3 outline_color  = vec3(0.0,0.0,1.0);
	const float outline_center = 0.58;
	vec3 glow_color     = vec3(1.0, 1.0, 0.0);
	const float glow_center    = 1.0;

	void main() {
		float dist = texture2D(sTexture, UV).a;
#ifdef GL_ES
		float smoothing = 1.0 / (vScale * 16.0);
		float alpha = smoothstep(glyph_center-smoothing, glyph_center+smoothing, dist);
#else
		float width = fwidth(dist);
		float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);
		//float alpha = dist;
#endif

		//gl_FragColor = vec4(1.0, 0.0, 0.0, alpha);
		//vec3 rgb = mix(vec3(0,0,0), vec3(1.0,0.0,0.0), dist);
		//gl_FragColor = vec4(rgb, 1.0);//floor(dist + 0.500));
		gl_FragColor = vec4(color.rgb, color.a * alpha);
		//gl_FragColor = vec4(1.0, 0.0, 0.0, floor(dist + 0.500));
		//gl_FragColor += vec4(0.0, 1.0, 0.0, floor(dist + 0.533));

		//float mu = smoothstep(outline_center-width, outline_center+width, dist);
		//vec3 rgb = mix(outline_color, glyph_color, mu);
		//gl_FragColor = vec4(rgb, max(alpha,mu));

		//vec3 rgb = mix(glow_color, vec3(1.0,1.0,1.0), alpha);
		//float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
		//gl_FragColor = vec4(rgb, mu);//max(alpha,mu));

	}
