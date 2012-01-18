#ifndef __SXGE_SCENE_CAMERA_HH__
#define __SXGE_SCENE_CAMERA_HH__

#include "math/vmath.hh"

namespace sxge {

class Camera {
public:
	Camera() :
		eye(new vmath::vec3f(0, 0, 1)),
		up(new vmath::vec3f(0, 1, 1)),
		direction(new vmath::vec3f(0, 0, 0)),
		viewMatrix(new vmath::mat4f()) 
	{
		updateMatrix();	
	}

	Camera(vmath::vec3f *eye, vmath::vec3f *up,
		vmath::vec3f *direction) :
		eye(eye), up(up), direction(direction)
	{
		updateMatrix();
	}

	virtual ~Camera() {
		delete eye;
		delete up;
		delete direction;
		delete viewMatrix;
	}

	const vmath::mat4f& getMatrix() const {
		return *viewMatrix;
	}
	
protected:
	vmath::vec3f *eye;
	vmath::vec3f *up;
	vmath::vec3f *direction;
	vmath::mat4f *viewMatrix;

	void updateMatrix() {
		*viewMatrix = vmath::mat4f::lookAt(*eye, *direction, *up);
	}
};

} //namespace sxge

#endif //__SXGE_SCENE_CAMERA_HH__
