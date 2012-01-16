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
	info("OpenGL Vendor %s", vendor);
	info("OpenGL Renderer %s", renderer);
	info("OpenGL Version %s", version);
	info("GLSL Version %s", glslVersion);
}

class TestScreen : public sxge::Screen {
public:
	TestScreen() : shaderProg(NULL), ox(0), oy(0) {
	}

	~TestScreen() {
		if (shaderProg) {
			delete shaderProg;
		}
	}

	void init(void) {
		printGLInfo();
		shaderProg = new sxge::ShaderProgram();
		shaderProg->addShaderFromFile(sxge::Shader::Vertex, 
			"shaders/test.vert");
		shaderProg->addShaderFromFile(sxge::Shader::Fragment, 
			"shaders/test.frag");
		shaderProg->link();
	}

	void keyEvent(char key, SpecialKey sk, KeyStatus ks) {
		switch (sk) {
			case SK_Left:
				ox -= 0.1;
				break;
			case SK_Right:
				ox += 0.1;
				break;
			case SK_Up:
				oy += 0.1;
				break;
			case SK_Down:
				oy -= 0.1;
				break;
			default:
				break;
		}
	}
	void mouseEvent(unsigned x, unsigned y, MouseButton buttons) {
		dbg("%s:(%d,%d) %d", __func__, x, y, buttons);
	}
	void display(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		shaderProg->bind();
		drawPlane(0.5);
	}
	void reshape(unsigned width, unsigned height) {
		this->width = width;
		this->height = height;
		glViewport(0, 0, width, height);
	}
protected:
	sxge::ShaderProgram *shaderProg;
	unsigned width, height;

	double ox, oy;

	void drawPlane(float size) {
		GLfloat vertices[] = {
			0, 0,
			size, 0,
			size, size,
			0, size,
		};

		GLfloat colors[] = {
			1, 0, 0,
			0, 1, 0,
			0, 0, 1,
			0, 1, 1,
		};

		GLuint attr_pos, attr_col, mvp_mtx;
		GLuint pid = shaderProg->programID();

		attr_pos = glGetAttribLocation(pid, "position");
		attr_col = glGetAttribLocation(pid, "color");
		mvp_mtx = glGetUniformLocation(pid, "MVP");

		//FIXME: needs a reimplementation of window/screen
		double aspect = (double)width / height;
		auto proj = vmath::mat4f::projection(45.0, aspect, 1, 100);
		
		//Transformation matrix
		auto r3 = vmath::mat3f::rotateZ(3.14 / 9);
		auto translate = vmath::vec3f(ox, oy, -5);
		auto transform = vmath::mat4f(r3, translate);
		
		//Camera view matrix
		auto eye = vmath::vec3f(0, 0, 1);
		auto up = vmath::vec3f(0, 1, 1);
		auto to = vmath::vec3f(0, 0, 0);
		auto view = vmath::mat4f::lookAt(eye, to, up);

		auto mvp = proj * view * transform;

		glUniformMatrix4fv(mvp_mtx, 1, GL_FALSE, mvp.getData());

		glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(attr_col, 3, GL_FLOAT, GL_FALSE, 0, colors);

		glEnableVertexAttribArray(attr_pos);
		glEnableVertexAttribArray(attr_col);

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glDisableVertexAttribArray(attr_pos);
		glDisableVertexAttribArray(attr_col);
	}
};

int main() {
	TestScreen screen;
	sxge::EGL_X11_Window window(screen);
	return 0;
}
