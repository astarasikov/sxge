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
		viewMatrix(new vmath::mat4f()),
		isBad(true)
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

	const vmath::mat4f& getMatrix() {
		if (isBad) {
			updateMatrix();
		}

		return *viewMatrix;
	}

	const vmath::vec3f& eyeVector() {
		return *eye;
	} 

protected:
	vmath::vec3f *eye;
	vmath::vec3f *up;
	vmath::vec3f *direction;
	vmath::mat4f *viewMatrix;

	bool isBad;

	void v3f_around_v3f(vmath::vec3f &v1r,
		vmath::vec3f &v2r,
		float t)
	{
		vmath::vec3f v1 = v1r.norm();
		vmath::vec3f v2 = v2r.norm();
		vmath::vec3f c = v1.cross(v2);
		float a = v1.dot(v2);
		float X = a*v2.X() + (v1.X() - a * v2.X()) * std::cos(t)
			+ c.X() * sin(t);
		float Y = a*v2.Y() + (v1.Y() - a * v2.Y()) * std::cos(t)
			+ c.Z() * sin(t);
		float Z = a*v2.Z() + (v1.Z() - a * v2.Z()) * std::cos(t)
			+ c.Z() * sin(t);
		vmath::vec3f r(X, Y, Z);
		v1r = r;
	}

	void updateMatrix() {
		if (!isBad) {
			return;
		}
		*viewMatrix = vmath::mat4f::lookAt(*eye, *direction, *up);
		isBad = false;
	}
};

} //namespace sxge

#endif //__SXGE_SCENE_CAMERA_HH__
