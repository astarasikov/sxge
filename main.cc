#include <iostream>
#include "util/log.h"
#include "math/vmath.hh"
#include "opengl/shader.hh"
#include "opengl/program.hh"
#include "opengl/window_egl_x11.hh"
#include "opengl/screen.hh"

void printGLInfo(void) {
	auto renderer = glGetString(GL_RENDERER);
	auto vendor = glGetString(GL_VENDOR);
	auto version = glGetString(GL_VERSION);
	auto glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	printf("OpenGL Vendor %s\n", vendor);
	printf("OpenGL Renderer %s\n", renderer);
	printf("OpenGL Version %s\n", version);
	printf("GLSL Version %s\n", glslVersion);
}

class TestScreen : public sxge::Screen {
public:
	void init(void) {
		printGLInfo();
	}

	void keyEvent(char key, bool isDown) {
		printf("%s:%c:%d\n", __func__, key, isDown);
	}
	void mouseEvent(unsigned x, unsigned y, MouseButton buttons) {
		printf("%s:(%d,%d) %d\n", __func__, x, y, buttons);
	}
	void display(void) {
		//printf("%s\n", __func__);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0, 1.0, 0.0, 1.0);
	}
	void reshape(unsigned width, unsigned height) {
		printf("%s: %d %d", __func__, width, height);
		glViewport(0, 0, width, height);
	}
};

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

	TestScreen screen;
	sxge::EGL_X11_Window window(screen);

#if 0
	sxge::Shader vsh(sxge::Shader::ShaderType::Vertex, "");
	sxge::ShaderProgram shp;
	shp.addShaderFromSource(sxge::Shader::ShaderType::Vertex, "");
#endif

	return 0;
}
