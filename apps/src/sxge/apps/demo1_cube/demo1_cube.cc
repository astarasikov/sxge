#include <cassert>
#include <iostream>

#include <sxge/util/log.h>
#include <sxge/math/vmath.hh>
#include <sxge/math/quaternion.hh>
#include <sxge/math/utils.hh>
#include <sxge/opengl/screen.hh>
#include <sxge/opengl/texture.hh>
#include <sxge/scene/light.hh>
#include <sxge/scene/object.hh>
#include <sxge/apps/apps.hh>

#include <math.h>

#include <xf86drm.h>

extern "C" {
#include "vaapi-recorder.h"
}
#include <sys/stat.h>
#include <fcntl.h>

#define SHADER_PATH(name) (SXGE_TOPDIR "/shaders/" name)
#ifdef SXGE_USE_OPENGL
	#define VTX_SHADER "light_gl3.vert"
	#define FRAG_SHADER "light_gl3.frag"
#else
	#define VTX_SHADER "light.vert"
	#define FRAG_SHADER "light.frag"
#endif

#define FALLBACK_FRAGMENT_SHADER "#version 100\n" \
	"precision mediump float;\n" \
	"varying vec4 vert_color;\n" \
	"\n" \
	"void main(void)\n" \
	"{\n" \
	"	gl_FragColor = vert_color;\n" \
	"}\n"

#define FALLBACK_VERTEX_SHADER "#version 100\n" \
	"precision mediump float;\n" \
	"uniform mat4 MVP;\n" \
	"attribute vec4 position;\n" \
	"attribute vec4 color;\n" \
	"\n" \
	"varying vec4 vert_color;\n" \
	"\n" \
	"void main(void)\n" \
	"{\n" \
	"	gl_Position = MVP * position;\n" \
	"	vert_color = color;\n" \
	"}\n"

#define ogl(x) do { \
	x; \
	if (glGetError()) { \
		sxge_info("Error at %d in '%s'", __LINE__, __func__); \
        exit(-1); \
	} \
} while (0)

#define INVALID_GL_HANDLE(x) ((x) == GL_INVALID_INDEX)

static int getDrmFd(void)
{
    struct stat stat_card0 = {};
    if (stat("/dev/dri/card0", &stat_card0)) {
        perror("stat");
        goto fail;
    }

    for (int fd = 0; fd < 16; fd++) {
        char buf[200] = {};
        sprintf(buf, "/dev/fd/%d", fd);

        struct stat stat_cur = {};

        if (stat(buf, &stat_cur)) {
            sxge_err("fail at '%s'", buf);
            continue;
        }

        if (stat_cur.st_dev == stat_card0.st_dev) {
            sxge_info("found DRM fd=%d", fd);
            return fd;
        }
    }

fail:
    sxge_errs("Failed to get DRM fd");
    return -1;
}

static struct vaapi_recorder *g_Recorder;

static GLuint _fb_texture;

static EGLDisplay _egl_display;
static EGLContext _egl_context;
static EGLSurface _egl_surface;

static EGLImageKHR _egl_image;
static int _drm_fd;

static EGLint _mesa_name;
static EGLint _mesa_handle;
static EGLint _mesa_stride;

enum {
    FRAME_WIDTH = 1024,
    FRAME_HEIGHT = 720,
};

extern "C" void sxge_exportEglDisplayContextSurface(
            EGLDisplay display,
            EGLContext context,
            EGLSurface surface)
{
    _egl_display = display;
    _egl_context = context;
    _egl_surface = surface;
}

static void InitVaapiBridge(void)
{
    _drm_fd = getDrmFd();
    if (!g_Recorder) {
            g_Recorder = vaapi_recorder_create(_drm_fd,
                    FRAME_WIDTH, FRAME_HEIGHT, "/tmp/out.h264");
    }

    ogl(glGenTextures(1, &_fb_texture));
    sxge_info("%s: _fb_texture=%08x", __func__, _fb_texture);
    ogl(glActiveTexture(GL_TEXTURE0));
    ogl(glBindTexture(GL_TEXTURE_2D, _fb_texture));
    ogl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FRAME_WIDTH, FRAME_HEIGHT,
                0, GL_RGB, GL_UNSIGNED_BYTE, NULL));

    ogl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    ogl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    ogl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    ogl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    GLuint frameBuffer;
    ogl(glGenFramebuffers(1, &frameBuffer));
    ogl(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));
    sxge_info("%s: frameBuffer=%08x", __func__, frameBuffer);

    ogl(glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D,
                _fb_texture,
                0));

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        ogl(sxge_errs("failed to construct framebuffer"));
    }

    ogl(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));
    ogl(glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT));

    ogl(_egl_image = eglCreateImageKHR
        (_egl_display, _egl_context, EGL_GL_TEXTURE_2D_KHR,
         (EGLClientBuffer)(size_t)_fb_texture, NULL));
    if (!_egl_image) {
        sxge_errs("failed to create EGLImage");
        exit(-1);
    }
    sxge_info("%s: _egl_image=%p", __func__, _egl_image);

    EGLBoolean ok;
    ogl(ok = eglExportDRMImageMESA(
                _egl_display,
                _egl_image,
                &_mesa_name,
                &_mesa_handle,
                &_mesa_stride));
    if (!ok) {
        sxge_errs("failed to export DRM image");
    }
    else {
        sxge_info("name=%08x handle=%08x stride=%d",
                _mesa_name, _mesa_handle, _mesa_stride);
    }
}

static void CaptureFrame(void)
{
    if (g_Recorder) {
        int prime_fd = 0;
        if (drmPrimeHandleToFD(_drm_fd, _mesa_handle, DRM_CLOEXEC, &prime_fd))
        {
            sxge_errs("drmPrimeHandleToFD failed");
            exit(-1);
        }
        vaapi_recorder_frame(g_Recorder, prime_fd, FRAME_WIDTH * 4);
    }
}

namespace sxge {

void printGLInfo(void) {
	auto renderer = glGetString(GL_RENDERER);
	auto vendor = glGetString(GL_VENDOR);
	auto version = glGetString(GL_VERSION);
	auto glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	sxge_info("OpenGL Vendor %s", vendor);
	sxge_info("OpenGL Renderer %s", renderer);
	sxge_info("OpenGL Version %s", version);
	sxge_info("GLSL Version %s", glslVersion);
}

void gl_check(void) {
	GLenum err = glGetError();
	if (!err) {
		return;
	}
	switch(err) {
		case GL_INVALID_ENUM:
		sxge_infos("invalid enum");
		break;
		case GL_INVALID_VALUE:
		sxge_infos("invalid value");
		break;
		case GL_INVALID_OPERATION:
		sxge_infos("invalid operation");
		break;
		case GL_OUT_OF_MEMORY:
		sxge_infos("OOM");
		break;
	}
}

Demo1_Cube::Demo1_Cube() :
	mShaderProg(nullptr),
	mCamera(new sxge::Camera()),
	mScene(nullptr),
	mWidth(0), mHeight(0),
	mMouseX(-1), mMouseY(-1),
	mOX(0), mOY(0), mOZ(-5),
	mRX(30), mRY(30), mRZ(0),
	mBuffer(nullptr), mBufferSize(0),
	mProjView(nullptr),
	mPositionAttr(GL_INVALID_INDEX),
	mColorAttr(GL_INVALID_INDEX),
	mTexCoordAttr(GL_INVALID_INDEX),
	mTexUniform(GL_INVALID_INDEX),
	mMVPAttr(GL_INVALID_INDEX),
	mEyeUniform(GL_INVALID_INDEX),
	mLightUniform(GL_INVALID_INDEX),
	mNormalAttr(GL_INVALID_INDEX),
	mVao(GL_INVALID_INDEX),
	mVbo(GL_INVALID_INDEX),
	mIndexVbo(GL_INVALID_INDEX)
{
}

Demo1_Cube::~Demo1_Cube() {
	delete mScene;
	delete mShaderProg;
	delete mCamera;
}

void Demo1_Cube::init(void) {
	initScene();
	printGLInfo();
	mShaderProg = new sxge::ShaderProgram();
	#ifndef ANDROID
	mShaderProg->addShaderFromFile(sxge::Shader::Vertex,
		SHADER_PATH(VTX_SHADER));
	mShaderProg->addShaderFromFile(sxge::Shader::Fragment,
		SHADER_PATH(FRAG_SHADER));
	#else
	mShaderProg->addShaderFromSource(sxge::Shader::Vertex,
		FALLBACK_VERTEX_SHADER);
	mShaderProg->addShaderFromSource(sxge::Shader::Fragment,
		FALLBACK_FRAGMENT_SHADER);
	#endif
	GLuint pid = mShaderProg->programID();

	#ifdef SXGE_USE_OPENGL
	GLuint attribIdx = 0;
	ogl(glBindAttribLocation(pid, attribIdx++, "position"));
	ogl(glBindAttribLocation(pid, attribIdx++, "color"));
	ogl(glBindAttribLocation(pid, attribIdx++, "texcoord"));
	ogl(glBindAttribLocation(pid, attribIdx++, "normal"));
	ogl(glBindFragDataLocation(pid, 0, "out_color"));

	ogl(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	#endif

	ogl(glEnable(GL_CULL_FACE));
	ogl(glEnable(GL_DEPTH_TEST));
	ogl(glEnable(GL_BLEND));
	ogl(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	//ogl(glEnable(GL_TEXTURE_2D));

	mShaderProg->link();

	#define GET(type, var, name) do { \
		ogl(var = glGet ## type ## Location(pid, name)); \
		if (INVALID_GL_HANDLE(var)) { \
			sxge_info("%s INVALID", name); \
		} \
		else { \
			sxge_info("%s:%d", name, var); \
		} \
	} while (0)

	GET(Attrib, mPositionAttr, "position");
	GET(Attrib, mColorAttr, "color");
	GET(Attrib, mTexCoordAttr, "texcoord");
	GET(Attrib, mNormalAttr, "normal");

	GET(Uniform, mTexUniform, "sTexture");
	GET(Uniform, mMVPAttr, "MVP");

	GET(Uniform, mEyeUniform, "eye");
	GET(Uniform, mLightUniform, "light");

	ogl(glGenVertexArrays(1, &mVao));
	ogl(glBindVertexArray(mVao));
	ogl(glGenBuffers(1, &mVbo));
	ogl(glGenBuffers(1, &mIndexVbo));

    InitVaapiBridge();
}

void Demo1_Cube::keyEvent(char key, SpecialKey sk, KeyStatus ks) {
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

void Demo1_Cube::mouseEvent(float x, float y, MouseButton buttons) {
	float dx = x - mMouseX;
	float dy = y - mMouseY;
	auto direction = mCamera->directionVector();
	vmath::vec3f delta(dx / mWidth, dy / mHeight, 0.0f);
	auto new_direction = direction + delta;
	mCamera->setDirection(new_direction);

	mMouseX = x;
	mMouseY = y;

	(void)buttons;
}

void Demo1_Cube::display(void) {
	ogl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	ogl(glClearColor(1.0, 1.0, 1.0, 1.0));
	assert(mShaderProg->bind());
	drawScene();

	//XXX: no input yet. add some motion
	mRX = ((mRX - 1) % -360);

    CaptureFrame();
}

void Demo1_Cube::reshape(unsigned width, unsigned height) {
	mWidth = width;
	mHeight = height;
	ogl(glViewport(0, 0, mWidth, mHeight));
}

void Demo1_Cube::drawModel(sxge::Model *model, sxge::Texture *texture) {
	size_t bufferElementCount = model->getNumVertices();

	size_t vertexOffset = 0;
	size_t colorOffset = model->getVertexStride() * bufferElementCount;
	size_t texOffset = colorOffset + model->getColorStride() * bufferElementCount;
	size_t normalOffset = texOffset + model->getTexStride() * bufferElementCount;

	size_t bufferSize = normalOffset + 3 * bufferElementCount;
	if (bufferSize > mBufferSize) {
		if (mBuffer) {
			delete[] mBuffer;
		}
		mBuffer = new GLfloat[bufferSize];
		mBufferSize = bufferSize;
		std::fill(mBuffer, mBuffer + bufferSize, 0.0f);
	}

	std::copy(model->vertices,
		model->vertices + bufferElementCount * model->getVertexStride(),
		mBuffer + vertexOffset);
	std::copy(model->colors,
		model->colors + bufferElementCount * model->getColorStride(),
		mBuffer + colorOffset);
	std::copy(model->texCoords,
		model->texCoords + bufferElementCount * model->getTexStride(),
		mBuffer + texOffset);

	if (model->vertexNormals) {
		std::copy(model->vertexNormals,
			model->vertexNormals + bufferElementCount * 3,
			mBuffer + normalOffset);
	}

	ogl(glBindVertexArray(mVao));

	ogl(glBindBuffer(GL_ARRAY_BUFFER, mVbo));
	ogl(glBufferData(GL_ARRAY_BUFFER,
		sizeof(GLfloat) * mBufferSize, mBuffer, GL_STATIC_DRAW));

	if (model->hasIndices()) {
		ogl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo));
		ogl(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(GLfloat) * model->getNumIndices(),
			model->indices, GL_STATIC_DRAW));
	}

	ogl(glVertexAttribPointer(mPositionAttr, model->getVertexStride(),
		GL_FLOAT, GL_FALSE, 0,
		(GLvoid*)(sizeof(GLfloat) * vertexOffset)));

	if (!INVALID_GL_HANDLE(mColorAttr)) {
		ogl(glVertexAttribPointer(mColorAttr, model->getColorStride(),
			GL_FLOAT, GL_FALSE, 0,
			(GLvoid*)(sizeof(GLfloat) * colorOffset)));
	}

	if (!INVALID_GL_HANDLE(mTexCoordAttr)) {
		ogl(glVertexAttribPointer(mTexCoordAttr, model->getTexStride(),
			GL_FLOAT, GL_FALSE, 0,
			(GLvoid*)(sizeof(GLfloat) * texOffset)));
	}

	if (!INVALID_GL_HANDLE(mNormalAttr)) {
		if (model->vertexNormals) {
			ogl(glVertexAttribPointer(mNormalAttr, 3,
				GL_FLOAT, GL_FALSE, 0,
				(GLvoid*)(sizeof(GLfloat) * normalOffset)));
		}
	}

	ogl(glEnableVertexAttribArray(mPositionAttr));

	if (!INVALID_GL_HANDLE(mColorAttr)) {
		ogl(glEnableVertexAttribArray(mColorAttr));
	}
	if (!INVALID_GL_HANDLE(mTexCoordAttr)) {
		ogl(glEnableVertexAttribArray(mTexCoordAttr));
	}

	if (!INVALID_GL_HANDLE(mNormalAttr) && model->vertexNormals) {
		ogl(glEnableVertexAttribArray(mNormalAttr));
	}

	if (texture) {
		ogl(glUniform1i(mTexUniform, 0));
		texture->buffer(GL_TEXTURE0);
	}

	if (model->hasIndices()) {
		ogl(glDrawElements(GL_TRIANGLES, model->getNumIndices(),
			GL_UNSIGNED_INT, 0));
	}
	else {
		ogl(glDrawArrays(GL_TRIANGLES, 0, model->getNumVertices()));
	}

	if (!INVALID_GL_HANDLE(mNormalAttr) && model->vertexNormals) {
		ogl(glDisableVertexAttribArray(mNormalAttr));
	}
	ogl(glDisableVertexAttribArray(mPositionAttr));

	if (!INVALID_GL_HANDLE(mColorAttr)) {
		ogl(glDisableVertexAttribArray(mColorAttr));
	}

	if (!INVALID_GL_HANDLE(mTexCoordAttr)) {
		ogl(glDisableVertexAttribArray(mTexCoordAttr));
	}

	ogl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	ogl(glBindBuffer(GL_ARRAY_BUFFER, 0));
	ogl(glBindVertexArray(0));
}

void Demo1_Cube::drawObject(sxge::Object *object) {
	if (object->transform) {
		vmath::mat4f &pv = *mProjView;
		vmath::mat4f &xform = *(object->transform);
		auto mvp = pv * xform;
		//auto mvp = xform * pv;
		ogl(glUniformMatrix4fv(mMVPAttr, 1, GL_FALSE, mvp.getData()));
	}
	else {
		ogl(glUniformMatrix4fv(mMVPAttr, 1, GL_FALSE, mProjView->getData()));
	}

	ogl(glUniform3f(mEyeUniform, 0, 0, 1));
	ogl(glUniform3f(mLightUniform, 0, 1, -1));

	drawModel(object->model, object->texture);
}

void Demo1_Cube::initScene(void) {
	auto cube1 = new sxge::Object();
	cube1->model = sxge::Model::cube(0.3);
	#ifndef ANDROID
	cube1->texture = new sxge::Texture(SXGE_TOPDIR "/res/tex1.dat", 256, 256);
	#endif

	auto cube2 = new sxge::Object();
	cube2->model = sxge::Model::cube(1.0);
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

void Demo1_Cube::drawScene() {
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
	auto translate = vmath::vec3f(mOX, mOY, mOZ);
	auto transform = vmath::mat4f(rXrZrY, translate);
	auto mvp = proj * view * transform;
	mProjView = &mvp;

	for (auto object : mScene->objects) {
		drawObject(object);
	}
}

} //namespace sxge
