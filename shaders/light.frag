varying vec4 vert_color;
varying vec2 vert_texcoord;

uniform sampler2D sTexture;

void main(void)
{
	gl_FragColor = texture2D(sTexture, vert_texcoord);
}
