#ifndef __SXGE_SCENE_LIGHT_HH__
#define __SXGE_SCENE_LIGHT_HH__

#include <sxge/math/vmath.hh>

namespace sxge {

class Light {
public:
	Light() : redColor(1.0), greenColor(1.0), blueColor(1.0) {}

protected:
	vmath::vec3f *position;
	float redColor;
	float greenColor;
	float blueColor;
};

} //namespace sxge

#endif //__SXGE_SCENE_LIGHT_HH__
