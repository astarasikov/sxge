#ifndef __SXGE_SCENE_MODEL_HH__
#define __SXGE_SCENE_MODEL_HH__

#include <algorithm>
#include "math/vmath.hh"

namespace sxge {

class Model {
public:
	Model() : vertices(NULL), indices(NULL), colors(NULL), texCoords(NULL),
		nVertices(0), nIndices(0), vtxStride(0), colStride(0)
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

	size_t getNumIndices() const {
		return nIndices;
	}

	bool hasIndices() const {
		return nIndices && (indices != NULL);
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
		mdl->nVertices = 4;

		mdl->colors = colors;
		mdl->colStride = 3;

		return mdl;
	}

	static Model *cube(float size) {
		const size_t nvert = 8 * 3;
		//const size_t nidx = 36;
		const size_t nidx = 36;
		
		const float colors[nvert] = {
			0, 0, 0,
			0, 0, 1,
			0, 1, 0,
			0, 1, 1,
			1, 0, 0,
			1, 0, 1,
			1, 1, 0,
			1, 1, 1,
		};

		const int indices[nidx] = {
			//bottom
			0, 4, 5,
			0, 5, 1,
		
			//left
			2, 0, 1,
			2, 1, 3,
			
			//right
			4, 6, 7,
			4, 7, 5,
			
			//top
			7, 6, 2,
			3, 7, 2,

			//front
			1, 5, 7,
			1, 7, 3,
			
			//back
			6, 4, 2,
			2, 4, 0,
		};

		float hsize = size / 2;

		float vertices[nvert] = {
			-hsize, -hsize, -hsize,
			-hsize, -hsize, hsize,
			-hsize, hsize, -hsize,
			-hsize, hsize, hsize,
			hsize, -hsize, -hsize,
			hsize, -hsize, hsize,
			hsize, hsize, -hsize,
			hsize, hsize, hsize,
		};
		
		auto mvertices = new float[nvert];
		auto mcolors = new float[nvert];
		auto mindices = new unsigned[nidx];

		std::copy(vertices, vertices + nvert, mvertices);
		std::copy(colors, colors + nvert, mcolors);
		std::copy(indices, indices + nidx, mindices);

		Model *mdl = new Model();
		mdl->vertices = mvertices;
		mdl->vtxStride = 3;
		mdl->nVertices = 8;

		mdl->colors = mcolors;
		mdl->colStride = 3;

		mdl->indices = mindices;
		mdl->nIndices = nidx;

		return mdl;
	}

	float *vertices;
	unsigned *indices;
	float *colors;
	float *texCoords;

protected:
	size_t nVertices;
	size_t nIndices;

	size_t vtxStride;
	size_t colStride;
};

} //namespace sxge


#endif //__SXGE_SCENE_MODEL_HH__
