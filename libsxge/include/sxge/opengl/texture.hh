#ifndef __SXGE_OPENGL_TEXTURE_HH__
#define __SXGE_OPENGL_TEXTURE_HH__

#include <string>

#include <sxge/platform/opengl/gl_common.hh>
#include <sxge/math/utils.hh>
#include <sxge/util/log.h>
#include <sxge/util/file_loader.hh>

namespace sxge {

class Texture {
public:
	Texture(unsigned width, unsigned height, char *raw_data)
		: textureID(-1U), isLoaded(false), width(width), height(height)
	{
		if (!raw_data) {
			sxge_errs("Texture data is NULL");
			return;
		}
		init(raw_data);
	}

	//TODO: support multiple texture formats
	Texture(std::string filename, unsigned width, unsigned height)
		: textureID(-1U), isLoaded(false), width(width), height(height)
	{
		sxge::FileLoader loader(filename);
		init(loader.getData());
	}

	virtual ~Texture() {
		glDeleteTextures(1, &textureID);
	}

	GLuint getTextureID() const {
		return textureID;
	}

	bool buffer(GLenum texture) {
		glActiveTexture(texture);
		glBindTexture(GL_TEXTURE_2D, textureID);
		return false;
	}
protected:
	GLuint textureID;
	bool isLoaded;

	unsigned width;
	unsigned height;

	bool init(char *raw_data) {
		if (!raw_data) {
			sxge_errs("Texture data is NULL");
			return false;
		}

		if (!isPowerOfTwo(width) || !isPowerOfTwo(height)) {
			sxge_errs("Texture dimensions are not powers of two");
			return false;
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, raw_data);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return true;
	}
};

} //namespace sxge

#endif //__SXGE_OPENGL_TEXTURE_HH__
