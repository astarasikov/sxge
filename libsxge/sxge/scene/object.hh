#ifndef __SXGE_SCENE_OBJECT_HH__
#define __SXGE_SCENE_OBJECT_HH__

#include <memory>
#include <set>

#include <sxge/math/vmath.hh>
#include <sxge/opengl/texture.hh>
#include <sxge/scene/model.hh>

namespace sxge {

class Object;

//TODO: add accessors for fields

class Object {
public:
	typedef std::set<Object*> objectSet;

	Object()
		: model(NULL), texture(NULL), transform(NULL), subObjects(objectSet())
	{}

	virtual ~Object() {
		delete model;
		delete texture;
		delete transform;
		for(auto object : subObjects) {
			delete object;
		}
	}

	sxge::Model *model;
	sxge::Texture *texture;
	vmath::mat4f *transform;
	objectSet subObjects;
};

} //namespace sxge


#endif //__SXGE_SCENE_OBJECT_HH__
