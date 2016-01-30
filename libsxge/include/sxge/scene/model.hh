#ifndef __SXGE_SCENE_MODEL_HH__
#define __SXGE_SCENE_MODEL_HH__

#include <algorithm>
#include <sxge/math/vmath.hh>

#include <tiny_obj_loader/tiny_obj_loader.h>

namespace sxge {

class Model {
protected:
	size_t nVertices;
	size_t nIndices;

	size_t vtxStride;
	size_t colStride;
	size_t texStride;

	inline vmath::vec3f vectorForIndex(size_t index);
	void computeVertexNormals(void);

public:
	float *vertices;
	unsigned *indices;
	float *colors;
	float *texCoords;
	float *vertexNormals;

	Model() :
		nVertices(0), nIndices(0), vtxStride(0), colStride(0), texStride(0),
		vertices(NULL), indices(NULL), colors(NULL), texCoords(NULL),
		vertexNormals(NULL) {}

	virtual ~Model();
	size_t getNumVertices() const;
	size_t getVertexStride() const;
	size_t getColorStride() const;
	size_t getNumIndices() const;
	bool hasIndices() const;
	size_t getTexStride() const;
	bool hasTextures() const;

	static Model *modelFromTinyObjShape(tinyobj::shape_t *shape);
	static Model *loadObj(const char *fpath);
	static std::vector<Model*> loadObjModels(const char *fpath);

	static Model *surface(float size);
	static Model *cube(float size);
};

} //namespace sxge


#endif //__SXGE_SCENE_MODEL_HH__
