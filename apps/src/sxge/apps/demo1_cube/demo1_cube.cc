#include <cassert>
#include <iostream>
#include <vector>

#include <sxge/util/log.h>
#include <sxge/math/vmath.hh>
#include <sxge/math/quaternion.hh>
#include <sxge/math/utils.hh>
#include <sxge/opengl/screen.hh>
#include <sxge/opengl/texture.hh>
#include <sxge/scene/animation.hh>
#include <sxge/scene/light.hh>
#include <sxge/scene/object.hh>
#include <sxge/apps/apps.hh>

#include <math.h>

#define OBJ_MDL_PATH (SXGE_TOPDIR "/3rdparty/res/mdl/cat.obj")
#define OBJ_TEX_PATH (SXGE_TOPDIR "/3rdparty/res/mdl/cat_diff.tga")

#define MULTIOBJ_MDL_PATH (SXGE_TOPDIR "/3rdparty/res/mdl/CHALLENGER71.obj")

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
	} \
} while (0)

#define INVALID_GL_HANDLE(x) ((x) == GL_INVALID_INDEX)

namespace sxge {

#include <sys/time.h>
anitime_t AnimationWallTime(void)
{
	timeval t;
	gettimeofday(&t, NULL);
	return (t.tv_sec * 1000000) + t.tv_usec;
}

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
		default:break;
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
	mVsyncEstimator = (struct VsyncEstimator){};
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
	#if 1//ndef ANDROID
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

	animationInit();
}

struct Demo1_Cube::AnimationSetup {
	FloatAnimationTarget mXTarget;
	Animation mXAnimation;

	FloatAnimationTarget mYTarget;
	Animation mYAnimation;

	IntAnimationTarget mRotTarget;
	Animation mRotAnimation;

	Timeline mTimeline;

	AnimationSetup(Demo1_Cube *ptr)
		:
			mXTarget(FloatAnimationTarget(ptr->mOX)),
			mYTarget(FloatAnimationTarget(ptr->mOY)),
			mRotTarget(IntAnimationTarget(ptr->mRY)),
			mTimeline(Timeline())
	{
		const size_t ANIM_DURATION = (165ull) * 100ull * 1000ull;

		mXAnimation = {
			.time_start = 0,
			.time_end = ANIM_DURATION,
			.from = -1.0,
			.to = 1.0,
			.target = &mXTarget,
			.interpolator = &gEaseOutBounceInterpolator,
			//interpolator:&gLinearInterpolator,
		};
		mYAnimation = {
			.time_start = 0,
			.time_end = ANIM_DURATION,
			.from = -0.5,
			.to = 0.5,
			.target = &mYTarget,
			.interpolator = &gSineInterpolator,
		};
		mRotAnimation = {
			.time_start = 0,
			.time_end = ANIM_DURATION,
			.from = -30.0,
			.to = 270.0,
			.target = &mRotTarget,
			.interpolator = &gSineInterpolator,
		};

		mTimeline.animations.push_back(&mXAnimation);
		mTimeline.animations.push_back(&mYAnimation);
		mTimeline.animations.push_back(&mRotAnimation);
	}
};

void Demo1_Cube::animationInit(void)
{
	mAnimationSetup = new AnimationSetup(this);
}

void Demo1_Cube::animationTick(void)
{
	static anitime_t lastVsyncTime = 0;
	anitime_t thisVsyncTime = AnimationWallTime();
	anitime_t dt = thisVsyncTime - lastVsyncTime;

	mAnimationSetup->mTimeline.tick(mVsyncEstimator.predict(thisVsyncTime));

	lastVsyncTime = thisVsyncTime;

	#ifdef ANDROID
	//XXX: no input yet. add some motion
	mRX = ((mRX - 1) % -360);
	#endif
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
		case 'h':
			mOX -= dt;
			break;
		case 'j':
			mOY += dt;
			break;
		case 'k':
			mOY -= dt;
			break;
		case 'l':
			mOX += dt;
			break;
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
		default:
			break;
	}
}

void Demo1_Cube::mouseEvent(float x, float y, MouseButton buttons) {
	return;

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
	animationTick();
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
		//vmath::mat4f &xform = *(object->transform);
		//auto mvp = pv * xform;
		//auto mvp = xform * pv;
		auto mvp = pv;
		ogl(glUniformMatrix4fv(mMVPAttr, 1, GL_TRUE, mvp.getData()));
	}
	else {
		ogl(glUniformMatrix4fv(mMVPAttr, 1, GL_TRUE, mProjView->getData()));
	}

	ogl(glUniform3f(mEyeUniform, 0, 0, 1));
	ogl(glUniform3f(mLightUniform, 0, 1, -1));

	drawModel(object->model, object->texture);
}

void Demo1_Cube::initScene(void) {
	auto obj1 = new sxge::Object();
	obj1->model = sxge::Model::loadObj(OBJ_MDL_PATH);
	if (!obj1->model) {
		sxge_errs("failed to load Obj model");
		obj1->model = sxge::Model::cube(0.3);
	}

	#if 1//ndef ANDROID
	obj1->texture = new sxge::Texture(OBJ_TEX_PATH, 512, 1024);
	#endif

	auto rX = vmath::mat3f::rotateX(sxge::degToRad(25.0f));
	auto rZ = vmath::mat3f::rotateZ(sxge::degToRad(40.0f));
	auto rXrZ = rZ * rX;
	auto translate = vmath::vec3f(2.0, 1.5, 0.2);
	auto scale3 = vmath::mat3f::scale(1, 1, 1);
	auto translated = vmath::mat4f(scale3, translate);
	auto rot4 = vmath::mat4f(rXrZ);
	auto transform = rot4 * translated;

	obj1->transform = new vmath::mat4f(transform);

	mScene = new sxge::Scene();
	mScene->add(obj1);
	{
		auto obj2 = new sxge::Object();
		obj2->model = sxge::Model::cube(0.2);
		obj2->texture = new sxge::Texture(SXGE_TOPDIR "/res/tex1.dat", 256, 256);

		auto o2_translate = vmath::vec3f(0.0, 0.0, 0.0);
		auto o2_translate_mtx = vmath::mat4f(o2_translate);
		obj2->transform = new vmath::mat4f(o2_translate_mtx);
		mScene->add(obj2);
	}

	std::vector<sxge::Model*> multimodels = Model::loadObjModels(MULTIOBJ_MDL_PATH);
	for (auto mdl : multimodels) {
		auto obj = new sxge::Object();
		obj->transform = obj1->transform;
		obj->model = mdl;
		mScene->add(obj);
	}
}

void Demo1_Cube::drawScene() {
	float aspect = (float)mWidth / mHeight;
	auto proj = vmath::mat4f::projection(45.0, aspect, 1, 1000);
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
