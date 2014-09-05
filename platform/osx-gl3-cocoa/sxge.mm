#import <OpenGL/gl3.h>

#import <cassert>
#import <iostream>

#import <sxge/util/log.h>
#import <sxge/math/vmath.hh>
#import <sxge/math/quaternion.hh>
#import <sxge/math/utils.hh>
#import <sxge/opengl/program.hh>
#import <sxge/opengl/screen.hh>
#import <sxge/opengl/shader.hh>
#import <sxge/opengl/texture.hh>
#import <sxge/scene/camera.hh>
#import <sxge/scene/model.hh>
#import <sxge/scene/light.hh>
#import <sxge/scene/object.hh>
#import <sxge/scene/scene.hh>

#import "sxge_view.h"
#import "sxge_gl_controller.h"
#import "opengl_view.h"
#import <math.h>

#define VTX_SHADER "../../shaders/light_gl3.vert"
#define FRAG_SHADER "../../shaders/light_gl3.frag"

#define ogl(x) do { \
	x; \
	if (glGetError()) { \
		info("Error at %d in '%s'", __LINE__, __func__); \
	} \
} while (0)

#define INVALID_GL_HANDLE(x) ((x) == ((GLuint)-1))

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
		info("%s", "invalid enum");
		break;
		case GL_INVALID_VALUE:
		info("%s", "invalid value");
		break;
		case GL_INVALID_OPERATION:
		info("%s", "invalid operation");
		break;
		case GL_OUT_OF_MEMORY:
		info("%s", "OOM");
		break;
	}
}

class TestScreen : public sxge::Screen {
public:
	TestScreen() :
		mShaderProg(NULL),
		mCamera(new sxge::Camera()),
		mOX(0), mOY(0), mOZ(-5),
		mRX(30), mRY(30), mRZ(0),
		mBuffer(nullptr), mBufferSize(0)
	{
	}

	~TestScreen() {
		delete mScene;
		delete mShaderProg;
		delete mCamera;
	}

	void init(void) {
		initScene();
		printGLInfo();
		mShaderProg = new sxge::ShaderProgram();
		mShaderProg->addShaderFromFile(sxge::Shader::Vertex,
			VTX_SHADER);
		mShaderProg->addShaderFromFile(sxge::Shader::Fragment,
			FRAG_SHADER);
		mShaderProg->link();

		ogl(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		ogl(glEnable(GL_CULL_FACE));
		ogl(glEnable(GL_DEPTH_TEST));
		ogl(glEnable(GL_BLEND));
		ogl(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		//ogl(glEnable(GL_TEXTURE_2D));

		GLuint pid = mShaderProg->programID();

		GLuint attribIdx = 0;
		ogl(glBindAttribLocation(pid, attribIdx++, "position"));
		ogl(glBindAttribLocation(pid, attribIdx++, "color"));
		ogl(glBindAttribLocation(pid, attribIdx++, "texcoord"));
		ogl(glBindAttribLocation(pid, attribIdx++, "normal"));
		ogl(glBindFragDataLocation(pid, 0, "out_color"));

		#define GET(type, var, name) do { \
			ogl(var = glGet ## type ## Location(pid, name)); \
			if (INVALID_GL_HANDLE(var)) { \
				info("%s INVALID", name); \
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
		ogl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		ogl(glClearColor(1.0, 1.0, 1.0, 1.0));
		assert(mShaderProg->bind());
		drawScene();
	}

	void reshape(unsigned width, unsigned height) {
		mWidth = width;
		mHeight = height;
		ogl(glViewport(0, 0, mWidth, mHeight));
	}
protected:
	sxge::ShaderProgram *mShaderProg;
	sxge::Camera *mCamera;
	sxge::Scene *mScene;

	unsigned mWidth, mHeight;

	double mOX, mOY, mOZ;
	int mRX, mRY, mRZ;
	
	GLfloat *mBuffer;
	size_t mBufferSize;

	vmath::mat4f *mProjView;
	GLuint mPositionAttr;
	GLuint mColorAttr;
	GLuint mTexCoordAttr;
	GLuint mTexUniform;
	GLuint mMVPAttr;

	GLuint mEyeUniform;
	GLuint mLightUniform;
	GLuint mNormalAttr;

	GLuint mVao;
	GLuint mVbo;
	GLuint mIndexVbo;

	void drawModel(sxge::Model *model, sxge::Texture *texture) {
		size_t bufferElementCount = model->hasIndices() ?
			model->getNumIndices() : model->getNumVertices();

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
		}

		std::copy(model->vertices, model->vertices + bufferElementCount,
			mBuffer + vertexOffset);
		std::copy(model->colors, model->colors + bufferElementCount,
			mBuffer + colorOffset);
		std::copy(model->texCoords, model->texCoords + bufferElementCount,
			mBuffer + texOffset);

		if (model->vertexNormals) {
			std::copy(model->vertexNormals, model->vertexNormals + bufferElementCount,
				mBuffer + normalOffset);
		}

		ogl(glBindVertexArray(mVao));

		ogl(glBindBuffer(GL_ARRAY_BUFFER, mVbo));
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(GLfloat) * mBufferSize, mBuffer, GL_STATIC_DRAW);

		if (model->hasIndices()) {
			ogl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo));
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				sizeof(GLfloat) * model->getNumIndices(),
				model->indices, GL_STATIC_DRAW);
		}

		ogl(glVertexAttribPointer(mPositionAttr, model->getVertexStride(),
			GL_FLOAT, GL_FALSE, 0, (GLvoid*)vertexOffset));

		if (!INVALID_GL_HANDLE(mColorAttr)) {
			ogl(glVertexAttribPointer(mColorAttr, model->getColorStride(),
				GL_FLOAT, GL_FALSE, 0, (GLvoid*)colorOffset));
		}

		if (!INVALID_GL_HANDLE(mTexCoordAttr)) {
			ogl(glVertexAttribPointer(mTexCoordAttr, model->getTexStride(),
				GL_FLOAT, GL_FALSE, 0, (GLvoid*)texOffset));
		}

		if (!INVALID_GL_HANDLE(mNormalAttr)) {
			if (model->vertexNormals) {
				ogl(glVertexAttribPointer(mNormalAttr, 3,
					GL_FLOAT, GL_FALSE, 0, (GLvoid*)normalOffset));
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

	void drawObject(sxge::Object *object) {
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

	void initScene(void) {
		auto cube1 = new sxge::Object();
		cube1->model = sxge::Model::cube(1.0);
		cube1->texture = new sxge::Texture("../../res/tex1.dat", 256, 256);

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
};

@implementation SxgeView
{
	TestScreen screen_;
}

-(void)prepareOpenGL
{
	NSLog(@"initializeContext");
	//screen_.init();
}

-(BOOL)acceptsFirstResponder {
	return YES;
}

-(void)keyDown:(NSEvent *)theEvent {
	NSLog(@"Key Event %@", theEvent);
	NSString *characters = [theEvent characters];
	if ([characters length] != 1) {
		return;
	}
	screen_.keyEvent([characters characterAtIndex:0],
		TestScreen::SK_None, TestScreen::KS_Down);
}

-(void)renderForTime:(CVTimeStamp)time
{
	NSLog(@"Render");
	if ([self lockFocusIfCanDraw] == NO) {
		return;
	}
	id context = [self openGLContext];
	CGLContextObj contextObj = (CGLContextObj)[context CGLContextObj];
	if (kCGLNoError != CGLLockContext(contextObj)) {
		goto fail_cg_lock;
	}

	static int done = 0;
	if (!done) {
		screen_.init();
		done = 1;
	}

	screen_.reshape(self.frame.size.width, self.frame.size.height);
	screen_.display();

	[context flushBuffer];

	CGLUnlockContext(contextObj);
fail_cg_lock:
	[self unlockFocus];
}
@end

int main(int argc, char** argv) {
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	NSApplication *app = [NSApplication sharedApplication];
	GLController *controller = [[GLController alloc] init];
	[NSApp setDelegate:controller];
	[app run];
	[pool release];
	return 0;
}
