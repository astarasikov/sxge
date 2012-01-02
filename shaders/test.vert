uniform mat4 MVP;

attribute vec4 position;
attribute vec4 color;

varying vec4 vert_color;

void main(void)
{
	gl_Position = MVP * position;
	vert_color = color;
}
