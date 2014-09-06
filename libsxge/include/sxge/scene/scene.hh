#ifndef __SXGE_SCENE_SCENE_HH__
#define __SXGE_SCENE_SCENE_HH__

#include <set>

#include <sxge/math/vmath.hh>
#include <sxge/scene/light.hh>
#include <sxge/scene/object.hh>

namespace sxge {

class Scene {
public:
	typedef std::set<sxge::Object*> objectSet;
	Scene() : objects(objectSet()){}
	virtual ~Scene() {
		for (auto it : objects) {
			delete it;
		}
	}

	void add(sxge::Object *object) {
		objects.insert(object);
	}

	objectSet objects;
};

} //namespace sxge

#endif //__SXGE_SCENE_SCENE_HH__
