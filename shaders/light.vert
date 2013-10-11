#version 100
precision mediump float;

uniform mat4 MVP;
attribute vec4 position;
attribute vec4 color;
attribute vec2 texcoord;

attribute vec3 normal;
uniform vec3 eye;
uniform vec3 light;

varying vec4 vert_color;
varying vec2 vert_texcoord;

void main(void)
{
	gl_Position = MVP * position;
	vert_texcoord = texcoord;
	
	vec3 normal_t = mat3(MVP) * normal;

	vec3 lv = normalize(light - gl_Position.xyz);
	vec3 ev = normalize(eye - gl_Position.xyz);
	vec3 ref = 2.0 * dot(normal_t, lv) * normal_t - lv;
	float blinn = max(0.0, dot(ref, ev));
	
//	vert_color = vec4(light, 1.0);
	vert_color = vec4(blinn, blinn, blinn, 1.0);
}
