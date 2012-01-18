#include <iostream>
#include "util/log.h"
#include "math/vmath.hh"
#include "math/utils.hh"
#include "opengl/shader.hh"
#include "opengl/program.hh"
#include "opengl/window_egl_x11.hh"
#include "opengl/screen.hh"
#include "scene/camera.hh"
#include "scene/model.hh"

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
	TestScreen() :
		shaderProg(NULL),
		camera(new sxge::Camera()),
		ox(0), oy(0), oz(-5),
		rx(0), ry(0), rz(0),
		mdl(sxge::Model::surface(1.0))
	{}

	~TestScreen() {
		delete shaderProg;
		delete camera;
		delete mdl;
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
		//FIXME: add proper camera, models, scene etc
		double dt = 0.1;

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
				rx = ((rx - 1) % -360);
				break;
			case 'X':
				rx = ((rx + 1) % 360);
				break;
			case 'y':
				ry = ((ry - 1) % -360);
				break;
			case 'Y':
				ry = ((ry + 1) % -360);
				break;
			case 'z':
				rz = ((rz - 1) % -360);
				break;
			case 'Z':
				rz = ((rz + 1) % -360);
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
		drawScene();
	}
	void reshape(unsigned width, unsigned height) {
		this->width = width;
		this->height = height;
		glViewport(0, 0, width, height);
	}
protected:
	sxge::ShaderProgram *shaderProg;
	sxge::Camera *camera;

	unsigned width, height;

	double ox, oy, oz;
	int rx, ry, rz;
	
	sxge::Model *mdl;

	void drawScene() {
		GLuint attr_pos, attr_col, mvp_mtx;
		GLuint pid = shaderProg->programID();

		attr_pos = glGetAttribLocation(pid, "position");
		attr_col = glGetAttribLocation(pid, "color");
		mvp_mtx = glGetUniformLocation(pid, "MVP");

		//FIXME: needs a reimplementation of window/screen
		double aspect = (double)width / height;
		auto proj = vmath::mat4f::projection(45.0, aspect, 1, 100);
		
		//Transformation matrix
		auto rX = vmath::mat3f::rotateX(sxge::degToRad((float)rx));
		auto rZ = vmath::mat3f::rotateZ(sxge::degToRad((float)rz));
		auto rY = vmath::mat3f::rotateY(sxge::degToRad((float)ry));
		auto rXrZ = rZ * rX;
		auto rXrZrY = rY * rXrZ;
		auto translate = vmath::vec3f(ox, oy, oz);
		auto transform = vmath::mat4f(rXrZrY, translate);
		auto view = camera->getMatrix();

		auto mvp = proj * view * transform;

		glUniformMatrix4fv(mvp_mtx, 1, GL_FALSE, mvp.getData());

		glVertexAttribPointer(attr_pos, mdl->getVertexStride(),
			GL_FLOAT, GL_FALSE, 0, mdl->vertices);

		glVertexAttribPointer(attr_col, mdl->getColorStride(),
			GL_FLOAT, GL_FALSE, 0, mdl->colors);

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
