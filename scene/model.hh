#ifndef __SXGE_SCENE_MODEL_HH__
#define __SXGE_SCENE_MODEL_HH__

#include <algorithm>
#include "math/vmath.hh"

namespace sxge {

class Model {
public:
	Model() : vertices(NULL), colors(NULL), texCoords(NULL),
		nVertices(0), nColors(0), nTexCoords(0),
		vtxStride(0), colStride(0)
	{}

	virtual ~Model() {
	}

	size_t getNumVertices() const {
		return nVertices;
	}

	size_t getVertexStride() const {
		return vtxStride;
	}

	size_t getColorStride() const {
		return colStride;
	}

	static Model *surface(float size) {
		Model *mdl = new Model();
		float half = size / 2;

		const size_t nvert = 8;
		const size_t ncol = 12;
		
		auto vertices = new float[nvert];
		auto colors = new float[ncol];

		std::fill(vertices, vertices + nvert, -half);
		std::fill(colors, colors + ncol, 0.0);

		vertices[2] = half;
		vertices[4] = half;
		vertices[5] = half;
		vertices[7] = half;

		colors[0] = 1;
		colors[4] = 1;
		colors[8] = 1;
		colors[11] = 1;
		colors[12] = 1;

		mdl->vertices = vertices;
		mdl->vtxStride = 2;
		mdl->nVertices = nvert;

		mdl->colors = colors;
		mdl->nColors = ncol;
		mdl->colStride = 3;

		return mdl;
	}

	float *vertices;
	float *colors;
	float *texCoords;

protected:
	size_t nVertices;
	size_t nColors;
	size_t nTexCoords;

	size_t vtxStride;
	size_t colStride;
};

} //namespace sxge


#endif //__SXGE_SCENE_MODEL_HH__
