#include <iostream>
#include "util/log.h"
#include "math/vmath.hh"
#include "opengl/shader.hh"
#include "opengl/program.hh"
#include "opengl/window_egl_x11.hh"
#include "opengl/screen.hh"

int main() {
	float data[9] = {
		1, 2, 3,
		4, 5, 6,
		7, 8, 9,
	};

	vmath::mat3f m3(data, false);
	vmath::mat4f m4(m3);
	vmath::mat3f mx(m3);
	std::cout << m4 << std::endl;

	vmath::mat4f mid = vmath::mat4f::identity();
	vmath::mat4f r = m4.transposed() * mid;
	std::cout << r << std::endl;

	sxge::EGL_X11_Window screen;

#if 0
	sxge::Shader vsh(sxge::Shader::ShaderType::Vertex, "");
	sxge::ShaderProgram shp;
	shp.addShaderFromSource(sxge::Shader::ShaderType::Vertex, "");
#endif

	return 0;
}
