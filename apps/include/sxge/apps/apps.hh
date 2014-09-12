#ifndef __SXGE_APPS_APPS__HH__
#define __SXGE_APPS_APPS__HH__

#include <sxge/opengl/screen.hh>
#include <sxge/opengl/program.hh>
#include <sxge/opengl/shader.hh>
#include <sxge/scene/camera.hh>
#include <sxge/scene/model.hh>
#include <sxge/scene/scene.hh>

namespace sxge {

class Demo1_Cube : public sxge::Screen {
public:
	Demo1_Cube();
	virtual ~Demo1_Cube();
	virtual void init(void) override;
	void keyEvent(char key, SpecialKey sk, KeyStatus ks) override;
	void mouseEvent(float x, float y, MouseButton buttons) override;
	void display(void) override;
	void reshape(unsigned width, unsigned height) override;
protected:
	void drawModel(sxge::Model *model, sxge::Texture *texture);
	void drawObject(sxge::Object *object);
	void initScene(void);
	void drawScene();

	sxge::ShaderProgram *mShaderProg;
	sxge::Camera *mCamera;
	sxge::Scene *mScene;

	unsigned mWidth, mHeight;

	float mMouseX, mMouseY;
	float mOX, mOY, mOZ;
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

};

} //namespace sxge

#endif
