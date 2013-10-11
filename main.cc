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
#include "scene/object.hh"
#include "scene/scene.hh"

#define VTX_SHADER "shaders/light.vert"
#define FRAG_SHADER "shaders/light.frag"

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
		mShaderProg(NULL),
		mCamera(new sxge::Camera()),
		mOX(0), mOY(0), mOZ(-5),
		mRX(30), mRY(30), mRZ(0)
	{
		initScene();
	}

	~TestScreen() {
		delete mScene;
		delete mShaderProg;
		delete mCamera;
	}

	void init(void) {
		printGLInfo();
		mShaderProg = new sxge::ShaderProgram();
		mShaderProg->addShaderFromFile(sxge::Shader::Vertex, 
			VTX_SHADER);
		mShaderProg->addShaderFromFile(sxge::Shader::Fragment, 
			FRAG_SHADER);
		mShaderProg->link();

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
	}

	void keyEvent(char key, SpecialKey sk, KeyStatus ks) {
		//FIXME: add proper camera, models, scene etc
		double dt = 0.1;

		switch (sk) {
			case SK_Left:
				mOX -= dt;
				break;
			case SK_Right:
				mOX += dt;
				break;
			case SK_Up:
				if (ks & KS_Ctrl) {
					mOZ += dt;
				}
				else {
					mOY += dt;
				}
				break;
			case SK_Down:
				if (ks & KS_Ctrl) {
					mOZ -= dt;
				}
				else {
					mOY -= dt;
				}
				break;
			default:
				break;
		}
		switch (key) {
			case 'x':
				mRX = ((mRX - 1) % -360);
				break;
			case 'X':
				mRX = ((mRX + 1) % 360);
				break;
			case 'y':
				mRY = ((mRY - 1) % -360);
				break;
			case 'Y':
				mRY = ((mRY + 1) % -360);
				break;
			case 'z':
				mRZ = ((mRZ - 1) % -360);
				break;
			case 'Z':
				mRZ = ((mRZ + 1) % -360);
				break;
		}
	}
	void mouseEvent(unsigned x, unsigned y, MouseButton buttons) {
		dbg("%s:(%d,%d) %d", __func__, x, y, buttons);
	}
	void display(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		assert(mShaderProg->bind());
		drawScene();
	}

	void reshape(unsigned width, unsigned height) {
		mWidth = width;
		mHeight = height;
		glViewport(0, 0, mWidth, mHeight);
	}
protected:
	sxge::ShaderProgram *mShaderProg;
	sxge::Camera *mCamera;
	sxge::Scene *mScene;

	unsigned mWidth, mHeight;

	double mOX, mOY, mOZ;
	int mRX, mRY, mRZ;
	
	vmath::mat4f *mProjView;
	GLuint mPositionAttr;
	GLuint mColorAttr;
	GLuint mTexCoordAttr;
	GLuint mTexUniform;
	GLuint mMVPAttr;

	GLuint mEyeUniform;
	GLuint mLightUniform;
	GLuint mNormalAttr;
	
	void drawModel(sxge::Model *model, sxge::Texture *texture) {
		glVertexAttribPointer(mPositionAttr, model->getVertexStride(),
			GL_FLOAT, GL_FALSE, 0, model->vertices);

		glVertexAttribPointer(mColorAttr, model->getColorStride(),
			GL_FLOAT, GL_FALSE, 0, model->colors);

		glVertexAttribPointer(mTexCoordAttr, model->getTexStride(),
			GL_FLOAT, GL_FALSE, 0, model->texCoords);

		if (model->vertexNormals) {
			glVertexAttribPointer(mNormalAttr, 3,
				GL_FLOAT, GL_FALSE, 0, model->vertexNormals);
		}
		
		glEnableVertexAttribArray(mPositionAttr);
		glEnableVertexAttribArray(mColorAttr);
		glEnableVertexAttribArray(mTexCoordAttr);

		if (model->vertexNormals) {
			glEnableVertexAttribArray(mNormalAttr);
		}

		if (texture) {
			gl_check();
			//FIXME: make the texture store its buffered state
			texture->buffer(GL_TEXTURE0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->getTextureID());
			glUniform1i(mTexUniform, 0);
		}

		if (model->hasIndices()) {
			glDrawElements(GL_TRIANGLES, model->getNumIndices(),
				GL_UNSIGNED_INT, model->indices);
		}
		else {
			glDrawArrays(GL_TRIANGLES, 0, model->getNumVertices());
		}
		
		if (model->vertexNormals) {
			glDisableVertexAttribArray(mNormalAttr);
		}
		glDisableVertexAttribArray(mPositionAttr);
		glDisableVertexAttribArray(mColorAttr);
		glDisableVertexAttribArray(mTexCoordAttr);
	}

	void drawObject(sxge::Object *object) {
		if (object->transform) {
			vmath::mat4f &pv = *mProjView;
			vmath::mat4f &xform = *(object->transform);
			auto mvp = pv * xform;
			//auto mvp = xform * pv;
			glUniformMatrix4fv(mMVPAttr, 1, GL_FALSE, mvp.getData());
		}
		else {
			glUniformMatrix4fv(mMVPAttr, 1, GL_FALSE, mProjView->getData());
		}
		
		glUniform3f(mEyeUniform, 0, 0, 1);
		glUniform3f(mLightUniform, 0, 1, 1);

		drawModel(object->model, object->texture);
	}

	void initScene(void) {
		auto cube1 = new sxge::Object();
		cube1->model = sxge::Model::cube(1.0); 
		cube1->texture = new sxge::Texture("res/tex1.dat", 256, 256);

		auto cube2 = new sxge::Object();
		cube2->model = cube1->model;
		cube2->texture = cube1->texture;
		
		auto rX = vmath::mat3f::rotateX(sxge::degToRad(25.0f));
		auto rZ = vmath::mat3f::rotateZ(sxge::degToRad(40.0f));
		auto rXrZ = rZ * rX;
		auto translate = vmath::vec3f(2.0, 1.5, 0.4);
		auto scale3 = vmath::mat3f::scale(1, 1, 1);
		auto translated = vmath::mat4f(scale3, translate);
		auto rot4 = vmath::mat4f(rXrZ);
		auto transform = rot4 * translated;

		cube1->transform = new vmath::mat4f(transform);

		mScene = new sxge::Scene();
		mScene->add(cube1);
		mScene->add(cube2);
	}

	void drawScene() {
		GLuint pid = mShaderProg->programID();

		mPositionAttr = glGetAttribLocation(pid, "position");
		mColorAttr = glGetAttribLocation(pid, "color");
		mTexCoordAttr = glGetAttribLocation(pid, "texcoord");
		mTexUniform = glGetUniformLocation(pid, "sTexture");
		mMVPAttr = glGetUniformLocation(pid, "MVP");

		mEyeUniform = glGetUniformLocation(pid, "eye");
		mLightUniform = glGetUniformLocation(pid, "light");
		mNormalAttr = glGetAttribLocation(pid, "normal");
		
		double aspect = (double)mWidth / mHeight;
		auto proj = vmath::mat4f::projection(45.0, aspect, 1, 100);
		auto view = mCamera->getMatrix();
		
		//Transformation matrix -- FIXME: add picker to rotate
		//objects and add camera rotation
		auto rX = vmath::mat3f::rotateX(sxge::degToRad((float)mRX));
		auto rZ = vmath::mat3f::rotateZ(sxge::degToRad((float)mRZ));
		auto rY = vmath::mat3f::rotateY(sxge::degToRad((float)mRY));
		auto rXrZ = rZ * rX;
		auto rXrZrY = rY * rXrZ;
		auto translate = vmath::vec3f(mOX, mOY,mOZ);
		auto transform = vmath::mat4f(rXrZrY, translate);
		auto mvp = proj * view * transform;
		mProjView = &mvp;
		
		for (auto object : mScene->objects) {
			drawObject(object);
		}
	}
};

int main() {
	TestScreen screen;
	sxge::EGL_X11_Window window(screen);
	return 0;
}
