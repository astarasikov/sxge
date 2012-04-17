#ifndef __SXGE_OPENGL_TEXTURE_HH__
#define __SXGE_OPENGL_TEXTURE_HH__

#include "gl_common.hh"
#include "math/utils.hh"
#include "util/log.h"
#include "util/file_loader.hh"
#include "string.h"

namespace sxge {

class Texture {
public:
	Texture(unsigned width, unsigned height, char *raw_data)
		: textureID(0), width(width), height(height)
	{
		if (!raw_data) {
			err("Texture data is NULL");
			return;
		}
		init(raw_data);
	}

	//TODO: support multiple texture formats
	Texture(std::string filename, unsigned width, unsigned height)
		: textureID(0), width(width), height(height)
	{
		sxge::FileLoader loader(filename);
		init(loader.getData());
	}
	
	virtual ~Texture() {
		glDeleteTextures(1, &textureID);

		if (tex_data) {
			delete[] tex_data;
		}
	}

	GLuint getTextureID() const {
		return textureID;
	}

	bool buffer(GLenum texture) {
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, tex_data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glActiveTexture(texture);
		return false;
	}
protected:
	GLuint textureID;
	bool isLoaded;

	unsigned width;
	unsigned height;
	char *tex_data;

	bool init(char *raw_data) {
		if (!raw_data) {
			err("Texture data is NULL");
			return false;
		}

		if (!isPowerOfTwo(width) || !isPowerOfTwo(height)) {
			err("Texture dimensions are not powers of two");
			return false;
		}

		size_t texSize = width * height * 3;
		tex_data = new char[texSize];
		std::copy(raw_data, raw_data + texSize, tex_data);

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, raw_data);

		return true;
	}
};

} //namespace sxge

#endif //__SXGE_OPENGL_TEXTURE_HH__
