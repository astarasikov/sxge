#version 100
precision mediump float;

uniform mat4 MVP;
attribute vec4 position;
attribute vec4 color;
attribute vec2 texcoord;

varying vec4 vert_color;
varying vec2 vert_texcoord;

void main(void)
{
	gl_Position = MVP * position;
	vert_color = color;
	vert_texcoord = texcoord;
}

