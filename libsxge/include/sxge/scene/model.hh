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

	inline vmath::vec3f vectorForIndex(size_t index) {
		size_t offset = index * vtxStride;
		float x = vertices[offset];
		float y = vertices[offset + 1];
		float z = vertices[offset + 2];
		return vmath::vec3f(x, y, z);
	}

	void computeVertexNormals() {
		vertexNormals = new float[nVertices * 3];
		std::fill(vertexNormals, vertexNormals + nVertices * 3, 0);

		using namespace vmath;
		if (indices) {
			size_t num_faces = nIndices / 3; //assume triangles for now
			auto normals = new float[num_faces * 3];
			std::fill(normals, normals + 3 * num_faces, 0);

			//calculate normals for each face
			for (size_t i = 0; i < num_faces; i++) {
				//offset in the index array for the current face
				size_t idx = 3 * i;

				//vector v1: index[1] - index[0]
				//vector v2: index[2] - index[0]
				//normal: cross(v1, v2)

				vec3f p0 = vectorForIndex(indices[idx]);
				vec3f p1 = vectorForIndex(indices[idx + 1]);
				vec3f p2 = vectorForIndex(indices[idx + 2]);

				vec3f v1 = p1 - p0;
				vec3f v2 = p2 - p0;
				vec3f normal = (v1.cross(v2)).norm();

				normals[idx] = normal.X();
				normals[idx + 1] = normal.Y();
				normals[idx + 2] = normal.Z();

				//for each vertex in this face
				for (size_t vi = 0; vi < 3; vi++) {
					size_t vnorm_idx = indices[idx + vi] * 3;

					vertexNormals[vnorm_idx] += normals[idx];
					vertexNormals[vnorm_idx + 1] += normals[idx + 1];
					vertexNormals[vnorm_idx + 2] += normals[idx + 2];
				}
			}

			//normalize vertex normals
			for (size_t i = 0; i < nVertices; i++) {
				float x = vertexNormals[i * 3];
				float y = vertexNormals[i * 3 + 1];
				float z = vertexNormals[i * 3 + 2];
				vec3f v(x, y, z);
				vec3f n = v.norm();

				vertexNormals[i * 3] = n.X();
				vertexNormals[i * 3 + 1] = n.Y();
				vertexNormals[i * 3 + 2] = n.Z();
			}
			delete[] normals;
		}
		else {
			//XXX: aww
		}
	}

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

	virtual ~Model() {
		delete [] indices;
		delete [] colors;
		delete [] texCoords;
		delete [] vertexNormals;
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

	size_t getTexStride() const {
		return texStride;
	}

	bool hasTextures() const {
		return texCoords != NULL;
	}

	static Model *loadObj(const char *fpath) {
		//XXX: write .obj loader, drop tiny_obj_loader

		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err = tinyobj::LoadObj(shapes, materials, fpath, NULL);
		if (!err.empty()) {
			std::cerr << err << std::endl;
			return NULL;
		}

		if (shapes.size() == 0) {
			sxge_errs("no shapes loaded");
			return NULL;
		}

		tinyobj::shape_t &shape = shapes[0];
		tinyobj::mesh_t &mesh = shape.mesh;

		size_t vtx_count = mesh.positions.size();
		size_t idx_count = mesh.indices.size();
		size_t tc_count = mesh.texcoords.size();
		size_t vn_count = mesh.normals.size();

		float *mvertices = nullptr;
		unsigned *mindices = nullptr;
		float *mtexcoords = nullptr;
		float *mnormals = nullptr;

		if (!vtx_count) {
			std::cerr << "no vertices in the model" << std::endl;
			return NULL;
		}
		else {
			std::cout << "Vertex count: " << vtx_count << std::endl;
		}

		Model *mdl = new Model();

		mvertices = new float[vtx_count];
		std::copy(mesh.positions.begin(), mesh.positions.end(), mvertices);
		mdl->vertices = mvertices;
		mdl->vtxStride = 3;
		mdl->nVertices = vtx_count;

		if (idx_count) {
			mindices = new unsigned[idx_count];
			std::copy(mesh.indices.begin(), mesh.indices.end(), mindices);
			mdl->indices = mindices;
			mdl->nIndices = idx_count;
		}

		if (tc_count) {
			if ((tc_count * 3 / 2) != vtx_count) {
				std::cerr << "texcoord count " << tc_count
					<< " (" << ((tc_count) * 3 / 2) << ")"
					<< " != " << vtx_count << std::endl;
				//return NULL;
			}
			mtexcoords = new float[tc_count];
			std::copy(mesh.texcoords.begin(), mesh.texcoords.end(), mtexcoords);
			mdl->texCoords = mtexcoords;
			mdl->texStride = 2;
		}

		if (vn_count) {
			if (vn_count != vtx_count) {
				std::cerr << "normal count " << vn_count
					<< " != " << vtx_count << std::endl;
				//return NULL;
			}
			mnormals = new float[vn_count];
			std::copy(mesh.normals.begin(), mesh.normals.end(), mnormals);
			mdl->vertexNormals = mnormals;
		}

		return mdl;
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
		const size_t vtx_stride = 3;
		const size_t col_stride = vtx_stride;
		const size_t n_vertices = 8;
		const size_t tex_stride = 2;
		const size_t nvert = vtx_stride * n_vertices;
		const size_t ntexc = tex_stride * n_vertices;
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

		const float texcoords[ntexc] = {
			0, 0,
			0, 1,
			1, 0,
			1, 1,
			0, 0,
			0, 1,
			1, 0,
			1, 1,
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
		auto mtexcoords = new float[ntexc];

		std::copy(vertices, vertices + nvert, mvertices);
		std::copy(colors, colors + nvert, mcolors);
		std::copy(indices, indices + nidx, mindices);
		std::copy(texcoords, texcoords + ntexc, mtexcoords);

		Model *mdl = new Model();
		mdl->vertices = mvertices;
		mdl->vtxStride = vtx_stride;
		mdl->nVertices = n_vertices;

		mdl->colors = mcolors;
		mdl->colStride = col_stride;

		mdl->indices = mindices;
		mdl->nIndices = nidx;

		mdl->texCoords = mtexcoords;
		mdl->texStride = tex_stride;

		mdl->computeVertexNormals();

		return mdl;
	}
};

} //namespace sxge


#endif //__SXGE_SCENE_MODEL_HH__
