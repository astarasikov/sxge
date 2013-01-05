uniform mat4 MVP;
attribute vec4 position;
attribute vec4 color;
attribute vec2 texcoord;

attribute vec3 eyepos;
attribute vec4 diffuse;

varying vec4 vert_color;
varying vec2 vert_texcoord;

void main(void)
{
	gl_Position = MVP * position;
	vert_color = color;
	vert_texcoord = texcoord;
}
