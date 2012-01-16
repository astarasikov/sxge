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
	TestScreen() : shaderProg(NULL), ox(0), oy(0), oz(-5),
		rx(0), ry(0), rz(0)
	{
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

	void clampAngle(double &angle) {
		while (angle > 6.28) {
			angle -= 6.28;
		}
		while (angle < -6.28) {
			angle += 6.28;
		}
		if (angle < 0) {
			angle += 6.28;
		}
	}

	void keyEvent(char key, SpecialKey sk, KeyStatus ks) {
		//FIXME: add proper camera, models, scene etc
		double dt = 0.1;
		double dr = 2 * 0.314; //as bad as it can be 

		switch (sk) {
			case SK_Left:
				ox -= dt;
				break;
			case SK_Right:
				ox += dt;
				break;
			case SK_Up:
				if (ks & KS_Ctrl) {
					oz += dt;
				}
				else {
					oy += dt;
				}
				break;
			case SK_Down:
				if (ks & KS_Ctrl) {
					oz -= dt;
				}
				else {
					oy -= dt;
				}
				break;
			default:
				break;
		}
		switch (key) {
			case 'x':
				rx -= dr;
				clampAngle(rx);
				break;
			case 'X':
				rx += dr;
				clampAngle(rx);
				break;
			case 'y':
				ry -= dr;
				clampAngle(ry);
				break;
			case 'Y':
				ry += dr;
				clampAngle(ry);
				break;
			case 'z':
				rz -= dr;
				clampAngle(rz);
				break;
			case 'Z':
				rz += dr;
				clampAngle(rz);
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

	double ox, oy, oz;
	double rx, ry, rz;

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
		auto rX = vmath::mat3f::rotateX(rx);
		auto rZ = vmath::mat3f::rotateZ(rz);
		auto rY = vmath::mat3f::rotateY(ry);
		auto rXrZ = rZ * rX;
		auto rXrZrY = rY * rXrZ;
		auto translate = vmath::vec3f(ox, oy, oz);
		auto transform = vmath::mat4f(rXrZrY, translate);
		
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
