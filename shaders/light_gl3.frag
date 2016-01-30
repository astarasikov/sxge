#version 150 core
in vec4 vert_color;
in vec2 vert_texcoord;
out vec4 out_color;

uniform sampler2D sTexture;

void main(void)
{
	vec2 tc_t = vert_texcoord;
	float mag = dot(tc_t, tc_t);
	float coef = (mag - 0.25) * -4;
	vec4 tex_col = texture(sTexture, vert_texcoord);

	//out_color = mix(tex_col, vert_color, coef * coef);
	out_color = tex_col;
}
