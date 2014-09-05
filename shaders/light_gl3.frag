#version 150 core
in vec4 vert_color;
in vec2 vert_texcoord;
out vec4 out_color;

uniform sampler2D sTexture;

void main(void)
{
	out_color = texture(sTexture, vert_texcoord) * vert_color;
}
