#include <cassert>
#include <iostream>
#include "util/log.h"
#include "math/vmath.hh"
#include "math/quaternion.hh"
#include "math/utils.hh"
#include "opengl/program.hh"
#include "opengl/screen.hh"
#include "opengl/shader.hh"
#include "opengl/texture.hh"
#include "opengl/window_egl_x11.hh"
#include "scene/camera.hh"
#include "scene/model.hh"
#include "scene/light.hh"

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

void gl_check(void) {
	GLenum err = glGetError();
	if (!err) {
		return;
	}
	switch(err) {
		case GL_INVALID_ENUM:
		info("invalid enum");
		break;
		case GL_INVALID_VALUE:
		info("invalid value");
		break;
		case GL_INVALID_OPERATION:
		info("invalid operation");
		break;
		case GL_OUT_OF_MEMORY:
		info("OOM");
		break;
	}
}

class TestScreen : public sxge::Screen {
public:
	TestScreen() :
		shaderProg(NULL),
		camera(new sxge::Camera()),
		ox(0), oy(0), oz(-5),
		rx(30), ry(30), rz(0),
		mdl(sxge::Model::cube(1.0)),
		texture(new sxge::Texture("res/tex1.dat", 256, 256))
	{}

	~TestScreen() {
		delete shaderProg;
		delete camera;
		delete mdl;
		delete texture;
	}

	void init(void) {
		printGLInfo();
		shaderProg = new sxge::ShaderProgram();
		shaderProg->addShaderFromFile(sxge::Shader::Vertex, 
			"shaders/test.vert");
		shaderProg->addShaderFromFile(sxge::Shader::Fragment, 
			"shaders/test.frag");
		shaderProg->link();

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);

		texture->buffer(GL_TEXTURE0);
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
		assert(shaderProg->bind());
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
	sxge::Texture *texture;

	void drawScene() {
		GLuint attr_pos, attr_col, attr_tex, mvp_mtx;
		GLuint texUniform;
		GLuint pid = shaderProg->programID();

		attr_pos = glGetAttribLocation(pid, "position");
		attr_col = glGetAttribLocation(pid, "color");
		attr_tex = glGetAttribLocation(pid, "texcoord");
		texUniform = glGetUniformLocation(pid, "sTexture");
		mvp_mtx = glGetUniformLocation(pid, "MVP");

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

		glVertexAttribPointer(attr_tex, mdl->getTexStride(),
			GL_FLOAT, GL_FALSE, 0, mdl->texCoords);
		
		glEnableVertexAttribArray(attr_pos);
		glEnableVertexAttribArray(attr_col);
		glEnableVertexAttribArray(attr_tex);

		gl_check();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->getTextureID());
		glUniform1i(texUniform, 0);

		if (mdl->hasIndices()) {
			glDrawElements(GL_TRIANGLES, mdl->getNumIndices(),
				GL_UNSIGNED_INT, mdl->indices);
		}
		else {
			glDrawArrays(GL_TRIANGLES, 0, mdl->getNumVertices());
		}
		
		glDisableVertexAttribArray(attr_pos);
		glDisableVertexAttribArray(attr_col);
		glDisableVertexAttribArray(attr_tex);
	}
};

static void test_quat(void) {
	auto v3 = vmath::vec3f(1, 1, 9);
	auto qf = vmath::Quaternion<float>(v3, 0.93214);	
	auto qc = qf.conjugated();
	std::cout << qc.mag() << std::endl;
}

int main() {
	test_quat();

	TestScreen screen;
	sxge::EGL_X11_Window window(screen);
	return 0;
}
