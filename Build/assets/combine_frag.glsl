#version 110

uniform sampler2D	tex0;
uniform sampler2D	tex1;

void main()
{ 
	vec3 base = texture2D( tex0, gl_TexCoord[0].st).rgb;
	vec3 blur = texture2D( tex1, gl_TexCoord[0].st).rgb;
	vec3 glow = blur * blur * 4;
	gl_FragColor.rgb = (1-glow)*base+glow;
	gl_FragColor.a = 1.0;
}